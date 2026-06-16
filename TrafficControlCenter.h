#ifndef TRAFFICCONTROLCENTER_H
#define TRAFFICCONTROLCENTER_H

#include "PriorityQueue.h"
#include "LinkedList.h"
#include "Event.h"
#include "Vehicle.h"
#include "Intersection.h"
#include "UI.h"

inline bool CompareEventPriority(Event* const& a, Event* const& b) {
    if (a == nullptr || b == nullptr) return false;
    
    if (a->GetTime() != b->GetTime()) {
        return a->GetTime() < b->GetTime();
    }
    
    char typeA = a->GetType();
    char typeB = b->GetType();
    int priorityA = 0, priorityB = 0;
    
    if (typeA == 'X') priorityA = 4;
    else if (typeA == 'P') priorityA = 3;
    else if ((typeA == 'A' && a->GetVehicleID() == 0) || typeA == 'R') priorityA = 2;
    else if (typeA == 'A') priorityA = 1;
    
    if (typeB == 'X') priorityB = 4;
    else if (typeB == 'P') priorityB = 3;
    else if ((typeB == 'A' && b->GetVehicleID() == 0) || typeB == 'R') priorityB = 2;
    else if (typeB == 'A') priorityB = 1;
    
    return priorityA > priorityB;
}

class TrafficControlCenter {
private:
    PriorityQueue<Event*> eventQueue;
    LinkedList<Vehicle*> allVehicles;
    LinkedList<Vehicle*> completedVehicles;
    Intersection* intersections;
    int numIntersections;
    int currentTime;
    int totalVehiclesServed;
    int totalWaitingTime;
    int totalVehiclesCanceled;
    int totalAccidents;
    int totalRoadClosures;
    Vehicle** vehicleMap;
    int maxVehicleID;
    
    int switchingCost;
    int autoPromotionThreshold;
    int cancellationThreshold;
    bool reroutingEnabled;
    int** reroutingMap;
    int numLanesPerIntersection;
    
    int totalAutoPromotions;
    int totalAutoCancellations;
    int totalSignalSwitches;
    int totalEVCount;
    int totalPTCount;
    int totalNCCount;
    int totalFVCount;

    void Initialize(int numInters, int numLanesPerInters, int switchCost);
    bool LoadInputFile(const char* filename);
    void ParseReroutingMap(std::ifstream& file, int numInters);
    void HandleEventsAtCurrentTime();
    void HandleArrival(Event* event);
    void HandlePromotion(Event* event);
    void HandleCancellation(Event* event);
    void HandleAccident(Event* event);
    void HandleRoadClosure(Event* event);
    void LaneScheduling();
    void CrossingLoop();
    void CollectStatistics();
    void CalculateFinalStatistics();
    Vehicle* FindVehicle(int vehicleID);
    void RegisterVehicle(Vehicle* vehicle);
    void HandleAutoPromotions();
    void HandleAutoCancellations();
    void UpdateAccidentDurations();
    void UpdateRoadClosureDurations();

public:
    TrafficControlCenter();
    ~TrafficControlCenter();
    TrafficControlCenter(const TrafficControlCenter&) = delete;
    TrafficControlCenter& operator=(const TrafficControlCenter&) = delete;

    bool RunSimulation(const char* inputFilename, const char* outputFilename, class UI* ui = nullptr, int mode = 2);
    bool WriteOutputFile(const char* filename);
    int GetCurrentTime() const { return currentTime; }
    int GetCurrentTimeValue() const { return currentTime; }
    bool IsSimulationComplete() const;
    Intersection* GetIntersections() const { return intersections; }
    int GetNumIntersections() const { return numIntersections; }
};

#endif

