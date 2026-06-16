#include "TrafficControlCenter.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

TrafficControlCenter::TrafficControlCenter()
    : eventQueue(CompareEventPriority), intersections(nullptr), 
      numIntersections(0), currentTime(0), totalVehiclesServed(0),
      totalWaitingTime(0), totalVehiclesCanceled(0), totalAccidents(0),
      totalRoadClosures(0), vehicleMap(nullptr), maxVehicleID(0),
      switchingCost(0), autoPromotionThreshold(5), cancellationThreshold(10),
      reroutingEnabled(false), reroutingMap(nullptr), numLanesPerIntersection(4),
      totalAutoPromotions(0), totalAutoCancellations(0),
      totalSignalSwitches(0), totalEVCount(0), totalPTCount(0),
      totalNCCount(0), totalFVCount(0) {
}

TrafficControlCenter::~TrafficControlCenter() {
    Node<Vehicle*>* current = allVehicles.GetHead();
    while (current != nullptr) {
        delete current->data;
        current = current->next;
    }
    
    if (intersections != nullptr) {
        for (int i = 0; i < numIntersections; i++) {
            intersections[i].~Intersection();
        }
        ::operator delete(intersections);
    }
    
    while (!eventQueue.IsEmpty()) {
        Event* event = eventQueue.Dequeue();
        delete event;
    }
    
    if (vehicleMap != nullptr) delete[] vehicleMap;
    
    if (reroutingMap != nullptr) {
        for (int i = 0; i < numIntersections; i++) {
            if (reroutingMap[i] != nullptr) delete[] reroutingMap[i];
        }
        delete[] reroutingMap;
    }
}

void TrafficControlCenter::Initialize(int numInters, int numLanesPerInters, int switchCost) {
    if (numInters <= 0) {
        std::cerr << "[ERROR] Invalid number of intersections: " << numInters << std::endl;
        numIntersections = 0;
        intersections = nullptr;
        return;
    }
    
    numIntersections = numInters;
    numLanesPerIntersection = numLanesPerInters;
    switchingCost = switchCost;
    
    try {
        intersections = (Intersection*)::operator new(numInters * sizeof(Intersection));
        for (int i = 0; i < numInters; i++) {
            new (&intersections[i]) Intersection(i + 1, numLanesPerInters, switchCost);
        }
        maxVehicleID = 10000;
        vehicleMap = new Vehicle*[maxVehicleID + 1];
        for (int i = 0; i <= maxVehicleID; i++) {
            vehicleMap[i] = nullptr;
        }
    } catch (const std::bad_alloc& ex) {
        std::cerr << "[ERROR] Memory allocation failed in Initialize: " << ex.what() << std::endl;
        numIntersections = 0;
        intersections = nullptr;
        vehicleMap = nullptr;
    }
}

void TrafficControlCenter::ParseReroutingMap(std::ifstream& file, int numInters) {
    if (numInters <= 0) {
        std::cerr << "[ERROR] Invalid numInters in ParseReroutingMap: " << numInters << std::endl;
        return;
    }
    
    std::string line;
    std::getline(file, line);
    
    try {
        reroutingMap = new int*[numInters + 1];
        for (int i = 0; i <= numInters; i++) {
            reroutingMap[i] = new int[numInters + 1];
            for (int j = 0; j <= numInters; j++) {
                reroutingMap[i][j] = 0;
            }
        }
        
        int validLinesRead = 0;
        while (validLinesRead < numInters && file.good()) {
            std::getline(file, line);
            
            if (line.empty() || line.find("Connections:") != std::string::npos) {
                continue;
            }
            
            if (line.find(":") == std::string::npos) {
                continue;
            }
            
            std::istringstream iss(line);
            int intersectionID = 0;
            char colon = '\0';
            
            if (!(iss >> intersectionID >> colon)) {
                continue;
            }
            
            if (intersectionID < 1 || intersectionID > numInters) {
                std::cerr << "[WARNING] Invalid intersection ID in rerouting map: " << intersectionID << " (expected 1-" << numInters << ")" << std::endl;
                continue;
            }
            
            validLinesRead++;
            int connectedID;
            while (iss >> connectedID) {
                if (connectedID < 1 || connectedID > numInters) {
                    std::cerr << "[WARNING] Invalid connected ID in rerouting map: " << connectedID << " (expected 1-" << numInters << ")" << std::endl;
                    continue;
                }
                reroutingMap[intersectionID][connectedID] = 1;
                reroutingMap[connectedID][intersectionID] = 1;
            }
        }
        
        if (validLinesRead < numInters) {
            std::cerr << "[WARNING] Only read " << validLinesRead << " rerouting lines, expected " << numInters << std::endl;
        }
    } catch (const std::bad_alloc& ex) {
        std::cerr << "[ERROR] Memory allocation failed in ParseReroutingMap: " << ex.what() << std::endl;
        reroutingMap = nullptr;
    }
}

bool TrafficControlCenter::LoadInputFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open input file: " << filename << std::endl;
        return false;
    }
    
    int numInters;
    file >> numInters;
    file >> switchingCost;
    file >> autoPromotionThreshold;
    file >> cancellationThreshold;
    
    std::string reroutingFlag;
    file >> reroutingFlag;
    reroutingEnabled = (reroutingFlag == "ON");
    
    if (reroutingEnabled) {
        ParseReroutingMap(file, numInters);
    } else {
        std::string line;
        std::getline(file, line);
        std::getline(file, line);
        for (int i = 0; i < numInters; i++) {
            std::getline(file, line);
        }
    }
    
    int numEvents;
    file >> numEvents;
    
    std::string dummy;
    std::getline(file, dummy);
    
    std::cerr << "[DEBUG] Number of events: " << numEvents << std::endl;
    
    Initialize(numInters, 4, switchingCost);
    
    if (intersections == nullptr) {
        std::cerr << "[ERROR] Failed to initialize intersections!" << std::endl;
        file.close();
        return false;
    }
    
    std::cerr << "[DEBUG] Initialization complete." << std::endl;
    
    for (int i = 0; i < numEvents; i++) {
        std::string line;
        if (!std::getline(file, line)) {
            std::cerr << "[WARNING] End of file reached while reading events. Read " << i << " of " << numEvents << " events." << std::endl;
            break;
        }
        
        if (line.empty()) {
            i--;
            continue;
        }
        
        std::istringstream iss(line);
        std::string eventTypeStr;
        iss >> eventTypeStr;
        
        Event* event = nullptr;
        
        if (eventTypeStr == "A") {
            std::string subtypeStr;
            int time, vehicleID, intersectionID, laneID, param1, param2 = 0;
            iss >> subtypeStr >> time >> vehicleID >> intersectionID >> laneID >> param1;
            if (iss >> param2) {}
            
            char subtype = (subtypeStr.empty() ? 'N' : subtypeStr[0]);
            
            Vehicle* vehicle = nullptr;
            switch (subtype) {
                case 'E': 
                    vehicle = new EV(vehicleID, time, param1, intersectionID, laneID, param2);
                    break;
                case 'P': 
                    vehicle = new PT(vehicleID, time, param1, intersectionID, laneID);
                    break;
                case 'N': 
                    vehicle = new NC(vehicleID, time, param1, intersectionID, laneID);
                    break;
                case 'F': 
                    vehicle = new FV(vehicleID, time, param1, intersectionID, laneID);
                    break;
                default: 
                    vehicle = new NC(vehicleID, time, param1, intersectionID, laneID);
                    break;
            }
            if (vehicle != nullptr) {
                RegisterVehicle(vehicle);
                allVehicles.InsertEnd(vehicle);
                event = new Event(time, 'A', subtype, vehicleID, intersectionID, laneID, param1, param2);
                std::cerr << "[DEBUG] Created vehicle: ID=" << vehicleID << ", Type=" << subtypeStr << ", Time=" << time << std::endl;
            } else {
                std::cerr << "[ERROR] Failed to create vehicle for event " << i << std::endl;
            }
            
        } else if (eventTypeStr == "P") {
            int time, vehicleID;
            iss >> time >> vehicleID;
            event = new Event(time, 'P', vehicleID);
            
        } else if (eventTypeStr == "X") {
            int time, vehicleID;
            iss >> time >> vehicleID;
            event = new Event(time, 'X', vehicleID);
            
        } else if (eventTypeStr == "ACC") {
            int time, intersectionID, laneID, duration;
            iss >> time >> intersectionID >> laneID >> duration;
            event = new Event(time, 'A', intersectionID, laneID, duration);
            
        } else if (eventTypeStr == "RC") {
            int time, intersectionID, laneID, duration;
            iss >> time >> intersectionID >> laneID >> duration;
            event = new Event(time, 'R', intersectionID, laneID, duration);
        }
        
        if (event != nullptr) {
            eventQueue.Enqueue(event);
        }
    }
    
    file.close();
    return true;
}

void TrafficControlCenter::HandleEventsAtCurrentTime() {
    LinkedList<Event*> eventsAtTime;
    while (!eventQueue.IsEmpty()) {
        Event* event = eventQueue.Peek();
        if (event->GetTime() > currentTime) break;
        event = eventQueue.Dequeue();
        eventsAtTime.InsertEnd(event);
    }
    
    Node<Event*>* current = eventsAtTime.GetHead();
    while (current != nullptr) {
        Event* event = current->data;
        if (event->GetType() == 'X') {
            HandleCancellation(event);
            delete event;
        }
        current = current->next;
    }
    
    current = eventsAtTime.GetHead();
    while (current != nullptr) {
        Event* event = current->data;
        if (event->GetType() == 'P') {
            HandlePromotion(event);
            delete event;
        }
        current = current->next;
    }
    
    current = eventsAtTime.GetHead();
    while (current != nullptr) {
        Event* event = current->data;
        if (event->GetType() == 'A' && event->GetVehicleID() == 0) {
            HandleAccident(event);
            delete event;
        } else if (event->GetType() == 'R') {
            HandleRoadClosure(event);
            delete event;
        }
        current = current->next;
    }
    
    current = eventsAtTime.GetHead();
    while (current != nullptr) {
        Event* event = current->data;
        if (event->GetType() == 'A' && event->GetVehicleID() != 0) {
            HandleArrival(event);
            delete event;
        }
        current = current->next;
    }
    
    eventsAtTime.Clear();
}

void TrafficControlCenter::HandleArrival(Event* event) {
    int vehicleID = event->GetVehicleID();
    Vehicle* vehicle = FindVehicle(vehicleID);
    if (vehicle == nullptr) return;
    
    int intersectionID = vehicle->GetIntersectionID();
    int laneID = vehicle->GetLaneID();
    
    if (intersections != nullptr && intersectionID >= 1 && intersectionID <= numIntersections) {
        Intersection* intersection = &intersections[intersectionID - 1];
        Lane* lane = intersection->GetLane(laneID - 1);
        if (lane != nullptr) {
            vehicle->SetStatus(VehicleStatus::WAITING);
            lane->AddVehicle(vehicle);
        }
    }
}

void TrafficControlCenter::HandlePromotion(Event* event) {
    int vehicleID = event->GetVehicleID();
    Vehicle* vehicle = FindVehicle(vehicleID);
    if (vehicle == nullptr || vehicle->GetTypeChar() != 'P') return;
    
    int intersectionID = vehicle->GetIntersectionID();
    int laneID = vehicle->GetLaneID();
    
    if (intersections != nullptr && intersectionID >= 1 && intersectionID <= numIntersections) {
        Intersection* intersection = &intersections[intersectionID - 1];
        Lane* lane = intersection->GetLane(laneID - 1);
        if (lane != nullptr) {
            lane->RemoveVehicle(vehicle);
            
            EV* newEV = new EV(vehicleID, vehicle->GetArrivalTime(), 
                              vehicle->GetOriginalCrossingDuration(),
                              intersectionID, laneID, vehicle->GetWaitingTime());
            newEV->SetStatus(vehicle->GetStatus());
            
            vehicleMap[vehicleID] = newEV;
            
            Node<Vehicle*>* current = allVehicles.GetHead();
            while (current != nullptr) {
                if (current->data == vehicle) {
                    delete vehicle;
                    current->data = newEV;
                    break;
                }
                current = current->next;
            }
            
            lane->AddVehicle(newEV);
            totalAutoPromotions++;
        }
    }
}

void TrafficControlCenter::HandleCancellation(Event* event) {
    int vehicleID = event->GetVehicleID();
    Vehicle* vehicle = FindVehicle(vehicleID);
    if (vehicle == nullptr) return;
    
    vehicle->SetCanceled(true);
    int intersectionID = vehicle->GetIntersectionID();
    int laneID = vehicle->GetLaneID();
    
    if (intersections != nullptr && intersectionID >= 1 && intersectionID <= numIntersections) {
        Intersection* intersection = &intersections[intersectionID - 1];
        Lane* lane = intersection->GetLane(laneID - 1);
        if (lane != nullptr) {
            lane->RemoveVehicle(vehicle);
        }
    }
    totalVehiclesCanceled++;
}

void TrafficControlCenter::HandleAccident(Event* event) {
    int intersectionID = event->GetIntersectionID();
    int laneID = event->GetLaneID();
    int duration = event->GetParameter1();
    
    if (intersections != nullptr && intersectionID >= 1 && intersectionID <= numIntersections) {
        Intersection* intersection = &intersections[intersectionID - 1];
        Lane* lane = intersection->GetLane(laneID - 1);
        if (lane != nullptr) {
            int endTime = currentTime + duration;
            lane->SetBlocked(true, endTime);
            totalAccidents++;
        }
    }
}

void TrafficControlCenter::HandleRoadClosure(Event* event) {
    int intersectionID = event->GetIntersectionID();
    int laneID = event->GetLaneID();
    int duration = event->GetParameter1();
    
    if (intersections != nullptr && intersectionID >= 1 && intersectionID <= numIntersections) {
        Intersection* intersection = &intersections[intersectionID - 1];
        if (laneID == -1 || laneID == 0) {
            int endTime = currentTime + duration;
            intersection->SetClosed(true, endTime);
        } else {
            Lane* lane = intersection->GetLane(laneID - 1);
            if (lane != nullptr) {
                int endTime = currentTime + duration;
                lane->SetBlocked(true, endTime);
            }
        }
        totalRoadClosures++;
    }
}

void TrafficControlCenter::LaneScheduling() {
    if (intersections == nullptr) return;
    for (int i = 0; i < numIntersections; i++) {
        Intersection* intersection = &intersections[i];
        if (intersection->IsClosed()) continue;
        
        int scheduledLane = intersection->ScheduleLane(currentTime);
        if (scheduledLane != -1 && scheduledLane != intersection->GetActiveLane()) {
            intersection->SwitchLane(scheduledLane, currentTime);
            totalSignalSwitches++;
        }
    }
}

void TrafficControlCenter::CrossingLoop() {
    if (intersections == nullptr) return;
    for (int i = 0; i < numIntersections; i++) {
        Intersection* intersection = &intersections[i];
        if (intersection->IsClosed()) continue;
        
        for (int j = 0; j < intersection->GetNumLanes(); j++) {
            Lane* lane = intersection->GetLane(j);
            if (lane != nullptr) {
                Vehicle* crossing = lane->GetCurrentCrossing();
                if (crossing != nullptr) {
                    crossing->DecrementCrossingDuration();
                    
                    if (crossing->GetCrossingDuration() == 0) {
                        crossing->SetCompletionTime(currentTime);
                        crossing->SetCrossing(false);
                        crossing->SetStatus(VehicleStatus::COMPLETED);
                        crossing->UpdateWaitingTime(currentTime);
                        completedVehicles.InsertEnd(crossing);
                        
                        if (crossing->GetTypeChar() == 'F') {
                            lane->SetLocked(true, currentTime);
                        }
                        
                        lane->SetCurrentCrossing(nullptr);
                        
                        totalVehiclesServed++;
                        totalWaitingTime += crossing->GetWaitingTime();
                    }
                }
            }
        }
        
        intersection->HandleCrossing(currentTime);
    }
}

void TrafficControlCenter::CollectStatistics() {
    Node<Vehicle*>* current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* vehicle = current->data;
        if (vehicle != nullptr) {
            if (!vehicle->IsCanceled() && !vehicle->IsCrossing()) {
                vehicle->UpdateWaitingTime(currentTime);
            }
        }
        current = current->next;
    }
}

void TrafficControlCenter::CalculateFinalStatistics() {
    totalEVCount = 0;
    totalPTCount = 0;
    totalNCCount = 0;
    totalFVCount = 0;
    
    Node<Vehicle*>* current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* vehicle = current->data;
        if (vehicle != nullptr) {
            char type = vehicle->GetTypeChar();
            if (type == 'E') totalEVCount++;
            else if (type == 'P') totalPTCount++;
            else if (type == 'N') totalNCCount++;
            else if (type == 'F') totalFVCount++;
        }
        current = current->next;
    }
}

Vehicle* TrafficControlCenter::FindVehicle(int vehicleID) {
    if (vehicleID < 0 || vehicleID > maxVehicleID) return nullptr;
    return vehicleMap[vehicleID];
}

void TrafficControlCenter::RegisterVehicle(Vehicle* vehicle) {
    if (vehicle == nullptr) return;
    if (vehicleMap == nullptr) {
        std::cerr << "[ERROR] vehicleMap is null in RegisterVehicle!" << std::endl;
        return;
    }
    int id = vehicle->GetID();
    if (id < 0) {
        std::cerr << "[ERROR] Invalid vehicle ID: " << id << std::endl;
        return;
    }
    if (id > maxVehicleID) {
        try {
            Vehicle** newMap = new Vehicle*[id + 1];
            for (int i = 0; i <= maxVehicleID; i++) {
                newMap[i] = vehicleMap[i];
            }
            for (int i = maxVehicleID + 1; i <= id; i++) {
                newMap[i] = nullptr;
            }
            delete[] vehicleMap;
            vehicleMap = newMap;
            maxVehicleID = id;
        } catch (const std::bad_alloc& ex) {
            std::cerr << "[ERROR] Memory allocation failed in RegisterVehicle: " << ex.what() << std::endl;
            return;
        }
    }
    if (id <= maxVehicleID) {
        vehicleMap[id] = vehicle;
    }
}

bool TrafficControlCenter::RunSimulation(const char* inputFilename, const char* outputFilename, UI* ui, int mode) {
    if (inputFilename == nullptr) {
        std::cerr << "[ERROR] Input filename is null!" << std::endl;
        return false;
    }
    if (outputFilename == nullptr) {
        std::cerr << "[ERROR] Output filename is null!" << std::endl;
        return false;
    }
    
    try {
        std::cerr << "[DEBUG] Loading input file: " << inputFilename << std::endl;
        if (!LoadInputFile(inputFilename)) {
            std::cerr << "[ERROR] Failed to load input file: " << inputFilename << std::endl;
            return false;
        }
        
        std::cerr << "[DEBUG] Input file loaded successfully." << std::endl;
        std::cerr << "[DEBUG] Number of intersections: " << numIntersections << std::endl;
        std::cerr << "[DEBUG] Starting simulation..." << std::endl;
        
        int loopCount = 0;
        while (!IsSimulationComplete()) {
            loopCount++;
            if (loopCount % 100 == 0) {
                std::cerr << "[DEBUG] Timestep: " << currentTime << std::endl;
            }
            
            try {
                HandleEventsAtCurrentTime();
                HandleAutoPromotions();
                HandleAutoCancellations();
                UpdateAccidentDurations();
                UpdateRoadClosureDurations();
                LaneScheduling();
                CrossingLoop();
                CollectStatistics();
            } catch (const std::exception& ex) {
                std::cerr << "[ERROR] Exception in simulation loop at timestep " << currentTime << ": " << ex.what() << std::endl;
                return false;
            } catch (...) {
                std::cerr << "[ERROR] Unknown exception in simulation loop at timestep " << currentTime << "!" << std::endl;
                return false;
            }
            
            currentTime++;
            
            if (currentTime > 100000) {
                std::cerr << "[ERROR] Simulation timeout - too many timesteps!" << std::endl;
                return false;
            }
        }
        
        std::cerr << "[DEBUG] Simulation complete at timestep " << currentTime << ". Calculating statistics..." << std::endl;
        CalculateFinalStatistics();
        std::cerr << "[DEBUG] Writing output file..." << std::endl;
        return WriteOutputFile(outputFilename);
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Exception in RunSimulation: " << ex.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "[ERROR] Unknown exception in RunSimulation!" << std::endl;
        return false;
    }
}

bool TrafficControlCenter::WriteOutputFile(const char* filename) {
    if (filename == nullptr) {
        std::cerr << "[ERROR] Output filename is null!" << std::endl;
        return false;
    }
    
    std::string currentDirStr = "";
    try {
#ifdef _WIN32
        char currentDir[MAX_PATH];
        DWORD result = GetCurrentDirectoryA(MAX_PATH, currentDir);
        if (result > 0 && result < MAX_PATH) {
            currentDirStr = currentDir;
            std::cerr << "[DEBUG] Current working directory: " << currentDir << std::endl;
        }
#else
        char currentDir[4096];
        if (getcwd(currentDir, sizeof(currentDir))) {
            currentDirStr = currentDir;
            std::cerr << "[DEBUG] Current working directory: " << currentDir << std::endl;
        }
#endif
    } catch (...) {
    }
    
    std::cerr << "[DEBUG] Attempting to write output file: " << filename << std::endl;
    
    std::ofstream file(filename, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Cannot open output file: " << filename << std::endl;
        if (!currentDirStr.empty()) {
            std::cerr << "[ERROR] Full path would be: " << currentDirStr << "\\" << filename << std::endl;
        }
        std::cerr << "[ERROR] Check if directory exists and you have write permissions." << std::endl;
        return false;
    }
    
    std::cerr << "[DEBUG] File opened successfully. Writing data..." << std::endl;
    
    Node<Vehicle*>* temp = completedVehicles.GetHead();
    while (temp != nullptr) {
        Vehicle* v = temp->data;
        if (v != nullptr && v->IsCompleted()) {
            v->UpdateWaitingTime(currentTime);
        }
        temp = temp->next;
    }
    temp = allVehicles.GetHead();
    while (temp != nullptr) {
        Vehicle* v = temp->data;
        if (v != nullptr && v->IsCompleted()) {
            v->UpdateWaitingTime(currentTime);
        }
        temp = temp->next;
    }
    
    Node<Vehicle*>* current = completedVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* vehicle = current->data;
        if (vehicle != nullptr) {
            if (vehicle->IsCanceled()) {
                file << "-- " << vehicle->GetID() << " " 
                     << vehicle->GetArrivalTime() << " " 
                     << vehicle->GetWaitingTime() << " " 
                     << vehicle->GetOriginalCrossingDuration() << " ";
                char type = vehicle->GetTypeChar();
                if (type == 'E') file << "EV ";
                else if (type == 'P') file << "PT ";
                else if (type == 'N') file << "NC ";
                else if (type == 'F') file << "FV ";
                else file << type << " ";
                file << vehicle->GetIntersectionID() << " " 
                     << vehicle->GetLaneID() << " CANCELED" << std::endl;
            } else {
                file << vehicle->GetCompletionTime() << " " 
                     << vehicle->GetID() << " " 
                     << vehicle->GetArrivalTime() << " " 
                     << vehicle->GetWaitingTime() << " " 
                     << vehicle->GetOriginalCrossingDuration() << " ";
                char type = vehicle->GetTypeChar();
                if (type == 'E') file << "EV ";
                else if (type == 'P') file << "PT ";
                else if (type == 'N') file << "NC ";
                else if (type == 'F') file << "FV ";
                else file << type << " ";
                file << vehicle->GetIntersectionID() << " " 
                     << vehicle->GetLaneID() << std::endl;
            }
        }
        current = current->next;
    }
    
    current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* vehicle = current->data;
        if (vehicle != nullptr && vehicle->IsCompleted() && !vehicle->IsCanceled()) {
            bool alreadyWritten = false;
            Node<Vehicle*>* check = completedVehicles.GetHead();
            while (check != nullptr) {
                if (check->data == vehicle) {
                    alreadyWritten = true;
                    break;
                }
                check = check->next;
            }
            if (!alreadyWritten) {
                file << vehicle->GetCompletionTime() << " " 
                     << vehicle->GetID() << " " 
                     << vehicle->GetArrivalTime() << " " 
                     << vehicle->GetWaitingTime() << " " 
                     << vehicle->GetOriginalCrossingDuration() << " ";
                char type = vehicle->GetTypeChar();
                if (type == 'E') file << "EV ";
                else if (type == 'P') file << "PT ";
                else if (type == 'N') file << "NC ";
                else if (type == 'F') file << "FV ";
                else file << type << " ";
                file << vehicle->GetIntersectionID() << " " 
                     << vehicle->GetLaneID() << std::endl;
            }
        }
        current = current->next;
    }
    
    int totalVehicles = totalEVCount + totalPTCount + totalNCCount + totalFVCount;
    file << std::endl;
    file << "Total Vehicles: " << totalVehicles << std::endl;
    file << "Total EV: " << totalEVCount;
    if (totalAutoPromotions > 0) file << " (including " << totalAutoPromotions << " promotions)";
    file << std::endl;
    file << "Total PT: " << totalPTCount;
    if (totalAutoPromotions > 0) file << " (" << totalAutoPromotions << " promoted)";
    file << std::endl;
    file << "Total NC: " << totalNCCount;
    if (totalVehiclesCanceled > 0) file << " (" << totalVehiclesCanceled << " canceled)";
    file << std::endl;
    file << "Total FV: " << totalFVCount << std::endl;
    
    double avgWT = 0.0;
    if (totalVehiclesServed > 0) {
        avgWT = (double)totalWaitingTime / totalVehiclesServed;
    }
    file << "Average WT (all): " << std::fixed << std::setprecision(1) << avgWT << std::endl;
    
    int totalEVWT = 0;
    int evCount = 0;
    current = completedVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* v = current->data;
        if (v != nullptr && !v->IsCanceled() && v->GetTypeChar() == 'E') {
            totalEVWT += v->GetWaitingTime();
            evCount++;
        }
        current = current->next;
    }
    current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* v = current->data;
        if (v != nullptr && v->IsCompleted() && !v->IsCanceled() && v->GetTypeChar() == 'E') {
            bool alreadyCounted = false;
            Node<Vehicle*>* check = completedVehicles.GetHead();
            while (check != nullptr) {
                if (check->data == v) {
                    alreadyCounted = true;
                    break;
                }
                check = check->next;
            }
            if (!alreadyCounted) {
                totalEVWT += v->GetWaitingTime();
                evCount++;
            }
        }
        current = current->next;
    }
    double avgWTEV = 0.0;
    if (evCount > 0) {
        avgWTEV = (double)totalEVWT / evCount;
    }
    file << "Average WT (EV): " << std::fixed << std::setprecision(1) << avgWTEV << std::endl;
    
    int totalNCWT = 0;
    int ncCount = 0;
    current = completedVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* v = current->data;
        if (v != nullptr && !v->IsCanceled() && v->GetTypeChar() == 'N') {
            totalNCWT += v->GetWaitingTime();
            ncCount++;
        }
        current = current->next;
    }
    current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* v = current->data;
        if (v != nullptr && v->IsCompleted() && !v->IsCanceled() && v->GetTypeChar() == 'N') {
            bool alreadyCounted = false;
            Node<Vehicle*>* check = completedVehicles.GetHead();
            while (check != nullptr) {
                if (check->data == v) {
                    alreadyCounted = true;
                    break;
                }
                check = check->next;
            }
            if (!alreadyCounted) {
                totalNCWT += v->GetWaitingTime();
                ncCount++;
            }
        }
        current = current->next;
    }
    double avgWTNC = 0.0;
    if (ncCount > 0) {
        avgWTNC = (double)totalNCWT / ncCount;
    }
    file << "Average WT (NC): " << std::fixed << std::setprecision(1) << avgWTNC << std::endl;
    
    int totalXD = 0;
    int countXD = 0;
    current = completedVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* v = current->data;
        if (v != nullptr && !v->IsCanceled()) {
            totalXD += v->GetOriginalCrossingDuration();
            countXD++;
        }
        current = current->next;
    }
    current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* v = current->data;
        if (v != nullptr && v->IsCompleted() && !v->IsCanceled()) {
            bool alreadyCounted = false;
            Node<Vehicle*>* check = completedVehicles.GetHead();
            while (check != nullptr) {
                if (check->data == v) {
                    alreadyCounted = true;
                    break;
                }
                check = check->next;
            }
            if (!alreadyCounted) {
                totalXD += v->GetOriginalCrossingDuration();
                countXD++;
            }
        }
        current = current->next;
    }
    double avgXD = 0.0;
    if (countXD > 0) {
        avgXD = (double)totalXD / countXD;
    }
    file << "Average XD: " << std::fixed << std::setprecision(1) << avgXD << std::endl;
    
    int totalPTVehicles = totalPTCount + totalAutoPromotions;
    double promoPercent = 0.0;
    if (totalPTVehicles > 0) {
        promoPercent = (double)totalAutoPromotions * 100.0 / totalPTVehicles;
    }
    file << "% Auto-Promoted PT: " << std::fixed << std::setprecision(0) << promoPercent << "%" << std::endl;
    file << "Signal Switches: " << totalSignalSwitches << std::endl;
    
    double cancelPercent = 0.0;
    if (totalVehicles > 0) {
        cancelPercent = (double)totalVehiclesCanceled * 100.0 / totalVehicles;
    }
    file << "% Vehicles Canceled: " << std::fixed << std::setprecision(0) << cancelPercent << "%" << std::endl;
    
    file.flush();
    file.close();
    
    if (file.fail()) {
        std::cerr << "[ERROR] Failed to write to output file!" << std::endl;
        return false;
    }
    
    std::cerr << "[DEBUG] Output file written successfully: " << filename << std::endl;
    return true;
}

bool TrafficControlCenter::IsSimulationComplete() const {
    if (!eventQueue.IsEmpty()) return false;
    
    if (intersections == nullptr) return true;
    
    for (int i = 0; i < numIntersections; i++) {
        if (intersections[i].HasWaitingVehicles() || intersections[i].HasCrossingVehicles()) {
            return false;
        }
    }
    
    return true;
}

void TrafficControlCenter::HandleAutoPromotions() {
    Node<Vehicle*>* current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* vehicle = current->data;
        if (vehicle != nullptr && vehicle->GetTypeChar() == 'P' && 
            !vehicle->IsCanceled() && !vehicle->IsCrossing()) {
            vehicle->UpdateWaitingTime(currentTime);
            if (vehicle->GetWaitingTime() >= autoPromotionThreshold) {
                int intersectionID = vehicle->GetIntersectionID();
                int laneID = vehicle->GetLaneID();
                
    if (intersections != nullptr && intersectionID >= 1 && intersectionID <= numIntersections) {
        Intersection* intersection = &intersections[intersectionID - 1];
        Lane* lane = intersection->GetLane(laneID - 1);
        if (lane != nullptr) {
            lane->RemoveVehicle(vehicle);
            
            EV* newEV = new EV(vehicle->GetID(), vehicle->GetArrivalTime(),
                                          vehicle->GetOriginalCrossingDuration(),
                                          intersectionID, laneID, vehicle->GetWaitingTime());
                        newEV->SetStatus(vehicle->GetStatus());
                        
                        vehicleMap[vehicle->GetID()] = newEV;
                        delete vehicle;
                        current->data = newEV;
                        
                        lane->AddVehicle(newEV);
                        totalAutoPromotions++;
                    }
                }
            }
        }
        current = current->next;
    }
}

void TrafficControlCenter::HandleAutoCancellations() {
    Node<Vehicle*>* current = allVehicles.GetHead();
    while (current != nullptr) {
        Vehicle* vehicle = current->data;
        if (vehicle != nullptr && vehicle->GetTypeChar() == 'N' && 
            !vehicle->IsCanceled() && !vehicle->IsCrossing()) {
            vehicle->UpdateWaitingTime(currentTime);
            if (vehicle->GetWaitingTime() >= cancellationThreshold) {
                int intersectionID = vehicle->GetIntersectionID();
                int laneID = vehicle->GetLaneID();
                
                if (intersections != nullptr && intersectionID >= 1 && intersectionID <= numIntersections) {
                    Intersection* intersection = &intersections[intersectionID - 1];
                    Lane* lane = intersection->GetLane(laneID - 1);
                    if (lane != nullptr) {
                        vehicle->SetCanceled(true);
                        lane->RemoveVehicle(vehicle);
                        totalVehiclesCanceled++;
                        totalAutoCancellations++;
                    }
                }
            }
        }
        current = current->next;
    }
}

void TrafficControlCenter::UpdateAccidentDurations() {
    if (intersections == nullptr) return;
    for (int i = 0; i < numIntersections; i++) {
        Intersection* intersection = &intersections[i];
        for (int j = 0; j < intersection->GetNumLanes(); j++) {
            Lane* lane = intersection->GetLane(j);
            if (lane != nullptr && lane->IsBlocked() && lane->GetBlockEndTime() > 0) {
                if (currentTime >= lane->GetBlockEndTime()) {
                    lane->SetBlocked(false);
                }
            }
        }
    }
}

void TrafficControlCenter::UpdateRoadClosureDurations() {
    if (intersections == nullptr) return;
    for (int i = 0; i < numIntersections; i++) {
        Intersection* intersection = &intersections[i];
        if (intersection->IsClosed() && intersection->GetClosureEndTime() > 0) {
            if (currentTime >= intersection->GetClosureEndTime()) {
                intersection->SetClosed(false);
            }
        }
    }
}

