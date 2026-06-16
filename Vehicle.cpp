#include "Vehicle.h"

Vehicle::Vehicle() 
    : id(0), arrivalTime(0), waitingTime(0), crossingDuration(0),
      completionTime(0), intersectionID(0), laneID(0),
      status(VehicleStatus::WAITING), originalCrossingDuration(0) {
}

Vehicle::Vehicle(int vehicleID, int arrTime, int crossDur, int interID, int lID)
    : id(vehicleID), arrivalTime(arrTime), waitingTime(0),
      crossingDuration(crossDur), completionTime(0), intersectionID(interID),
      laneID(lID), status(VehicleStatus::WAITING), originalCrossingDuration(crossDur) {
}

Vehicle::~Vehicle() {
}

int Vehicle::GetID() const { return id; }
int Vehicle::GetArrivalTime() const { return arrivalTime; }
int Vehicle::GetWaitingTime() const { return waitingTime; }
int Vehicle::GetCrossingDuration() const { return crossingDuration; }
int Vehicle::GetOriginalCrossingDuration() const { return originalCrossingDuration; }
int Vehicle::GetCompletionTime() const { return completionTime; }
int Vehicle::GetIntersectionID() const { return intersectionID; }
int Vehicle::GetLaneID() const { return laneID; }
VehicleStatus Vehicle::GetStatus() const { return status; }
bool Vehicle::IsCanceled() const { return status == VehicleStatus::CANCELED; }
bool Vehicle::IsCrossing() const { return status == VehicleStatus::CROSSING; }
bool Vehicle::IsWaiting() const { return status == VehicleStatus::WAITING; }
bool Vehicle::IsCompleted() const { return status == VehicleStatus::COMPLETED; }

void Vehicle::SetIntersectionID(int id) { intersectionID = id; }
void Vehicle::SetLaneID(int id) { laneID = id; }
void Vehicle::SetCanceled(bool canceled) { 
    if (canceled) status = VehicleStatus::CANCELED;
}
void Vehicle::SetCrossing(bool crossing) { 
    if (crossing) {
        status = VehicleStatus::CROSSING;
    } else if (status == VehicleStatus::CROSSING) {
        status = VehicleStatus::WAITING;
    }
}
void Vehicle::SetCompletionTime(int time) { 
    completionTime = time;
    if (time > 0) status = VehicleStatus::COMPLETED;
}
void Vehicle::SetStatus(VehicleStatus newStatus) { status = newStatus; }
void Vehicle::SetCrossingDuration(int duration) { 
    crossingDuration = duration;
    if (originalCrossingDuration == 0) originalCrossingDuration = duration;
}

void Vehicle::DecrementCrossingDuration() {
    if (crossingDuration > 0 && status == VehicleStatus::CROSSING) {
        crossingDuration--;
    }
}

void Vehicle::UpdateWaitingTime(int currentTime) {
    if (status == VehicleStatus::WAITING && !IsCanceled()) {
        waitingTime = currentTime - arrivalTime;
        if (waitingTime < 0) waitingTime = 0;
    } else if (status == VehicleStatus::CROSSING && !IsCanceled()) {
        if (waitingTime < 0) {
            int timeSpentCrossing = originalCrossingDuration - crossingDuration;
            waitingTime = currentTime - arrivalTime - timeSpentCrossing;
            if (waitingTime < 0) waitingTime = 0;
        }
    } else if (status == VehicleStatus::COMPLETED && completionTime > 0) {
        waitingTime = completionTime - arrivalTime - originalCrossingDuration;
        if (waitingTime < 0) waitingTime = 0;
    }
}

void Vehicle::Print() const {
    std::cout << "Vehicle ID: " << GetID() << ", Type: " << GetTypeChar()
              << ", Arrival: " << GetArrivalTime() << ", Waiting: " << GetWaitingTime()
              << ", Intersection: " << GetIntersectionID() << ", Lane: " << GetLaneID();
    if (IsCanceled()) std::cout << " [CANCELED]";
    if (IsCrossing()) std::cout << " [CROSSING]";
    std::cout << std::endl;
}

EV::EV(int vehicleID, int arrTime, int crossDur, int interID, int lID, int priority)
    : Vehicle(vehicleID, arrTime, crossDur, interID, lID), priorityScore(priority) {
}

EV::~EV() {
}

int EV::GetPriorityScore() const { return priorityScore; }
void EV::SetPriorityScore(int score) { priorityScore = score; }

void EV::Print() const {
    std::cout << "Vehicle ID: " << GetID() << ", Type: E (Emergency)"
              << ", Arrival: " << GetArrivalTime() << ", Waiting: " << GetWaitingTime()
              << ", Priority: " << priorityScore
              << ", Intersection: " << GetIntersectionID() << ", Lane: " << GetLaneID();
    if (IsCanceled()) std::cout << " [CANCELED]";
    if (IsCrossing()) std::cout << " [CROSSING]";
    std::cout << std::endl;
}

PT::PT(int vehicleID, int arrTime, int crossDur, int interID, int lID)
    : Vehicle(vehicleID, arrTime, crossDur, interID, lID) {
}

PT::~PT() {
}

void PT::Print() const {
    std::cout << "Vehicle ID: " << GetID() << ", Type: P (Public Transport)"
              << ", Arrival: " << GetArrivalTime() << ", Waiting: " << GetWaitingTime()
              << ", Intersection: " << GetIntersectionID() << ", Lane: " << GetLaneID();
    if (IsCanceled()) std::cout << " [CANCELED]";
    if (IsCrossing()) std::cout << " [CROSSING]";
    std::cout << std::endl;
}

NC::NC(int vehicleID, int arrTime, int crossDur, int interID, int lID)
    : Vehicle(vehicleID, arrTime, crossDur, interID, lID) {
}

NC::~NC() {
}

void NC::Print() const {
    std::cout << "Vehicle ID: " << GetID() << ", Type: N (Normal Car)"
              << ", Arrival: " << GetArrivalTime() << ", Waiting: " << GetWaitingTime()
              << ", Intersection: " << GetIntersectionID() << ", Lane: " << GetLaneID();
    if (IsCanceled()) std::cout << " [CANCELED]";
    if (IsCrossing()) std::cout << " [CROSSING]";
    std::cout << std::endl;
}

FV::FV(int vehicleID, int arrTime, int crossDur, int interID, int lID)
    : Vehicle(vehicleID, arrTime, crossDur, interID, lID) {
}

FV::~FV() {
}

void FV::Print() const {
    std::cout << "Vehicle ID: " << GetID() << ", Type: F (Freight)"
              << ", Arrival: " << GetArrivalTime() << ", Waiting: " << GetWaitingTime()
              << ", Intersection: " << GetIntersectionID() << ", Lane: " << GetLaneID();
    if (IsCanceled()) std::cout << " [CANCELED]";
    if (IsCrossing()) std::cout << " [CROSSING]";
    std::cout << std::endl;
}

