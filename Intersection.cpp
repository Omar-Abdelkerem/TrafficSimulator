#include "Intersection.h"
#include "Vehicle.h"
#include <iostream>

Intersection::Intersection(int intersectionID, int numberOfLanes, int switchCost)
    : id(intersectionID), numLanes(numberOfLanes), activeLane(0),
      switchingCost(switchCost), isClosed(false), switchCounter(0), closureEndTime(-1) {
    lanes = new Lane[numberOfLanes];
}

Intersection::~Intersection() {
    delete[] lanes;
}

int Intersection::GetID() const { return id; }
int Intersection::GetNumLanes() const { return numLanes; }
int Intersection::GetActiveLane() const { return activeLane; }
Lane* Intersection::GetLane(int laneIndex) const {
    if (laneIndex < 0 || laneIndex >= numLanes) return nullptr;
    return &lanes[laneIndex];
}
bool Intersection::IsClosed() const { return isClosed; }
void Intersection::SetClosed(bool closed) { 
    isClosed = closed; 
    if (!closed) closureEndTime = -1;
}
void Intersection::SetClosed(bool closed, int endTime) { 
    isClosed = closed; 
    closureEndTime = endTime;
}
int Intersection::GetClosureEndTime() const { return closureEndTime; }

int Intersection::ScheduleLane(int currentTime) {
    if (isClosed) return -1;

    Lane* currentLane = GetLane(activeLane);
    if (currentLane != nullptr && currentLane->HasWaitingVehicles() && !currentLane->IsBlocked()) {
        if (currentLane->GetCurrentCrossing() == nullptr) {
            return activeLane;
        }
    }

    int bestLane = -1;
    int bestPriority = -1;
    
    for (int i = 0; i < numLanes; i++) {
        Lane* lane = GetLane(i);
        if (lane == nullptr || lane->IsBlocked()) continue;
        if (!lane->HasWaitingVehicles()) continue;

        if (lane->GetEVCount() > 0) {
            if (bestPriority < 4) { bestLane = i; bestPriority = 4; }
        } else if (lane->GetPTCount() > 0) {
            if (bestPriority < 3) { bestLane = i; bestPriority = 3; }
        } else if (lane->GetNCCount() > 0) {
            if (bestPriority < 2) { bestLane = i; bestPriority = 2; }
        } else if (lane->GetFVCount() > 0) {
            if (bestPriority < 1) { bestLane = i; bestPriority = 1; }
        }
    }

    if (bestLane != -1 && bestLane != activeLane) {
        if (bestPriority > 1 || currentLane == nullptr || !currentLane->HasWaitingVehicles()) {
            return bestLane;
        }
    }

    if (currentLane != nullptr && currentLane->HasWaitingVehicles() && !currentLane->IsBlocked()) {
        return activeLane;
    }

    return bestLane;
}

bool Intersection::SwitchLane(int newLaneIndex, int currentTime) {
    if (newLaneIndex < 0 || newLaneIndex >= numLanes) return false;
    if (newLaneIndex == activeLane) return true;
    activeLane = newLaneIndex;
    switchCounter++;
    return true;
}

void Intersection::HandleCrossing(int currentTime) {
    Lane* lane = GetLane(activeLane);
    if (lane == nullptr || lane->IsBlocked()) return;

    Vehicle* currentCrossing = lane->GetCurrentCrossing();
    if (currentCrossing == nullptr && lane->IsLocked()) {
        if (currentTime >= lane->GetLockTime() + 3) {
            lane->SetLocked(false, currentTime);
        } else {
            return;
        }
    }

    if (currentCrossing != nullptr) {
        currentCrossing->DecrementCrossingDuration();
        if (currentCrossing->GetCrossingDuration() <= 0) {
            currentCrossing->SetCompletionTime(currentTime);
            currentCrossing->SetStatus(VehicleStatus::COMPLETED);
            currentCrossing->UpdateWaitingTime(currentTime);
            if (currentCrossing->GetTypeChar() == 'F') {
                lane->SetLocked(true, currentTime);
            }
            lane->SetCurrentCrossing(nullptr);
        }
        return;
    }

    Vehicle* nextVehicle = lane->SelectNextVehicle();
    if (nextVehicle != nullptr) {
        nextVehicle->UpdateWaitingTime(currentTime);
        nextVehicle->SetCrossing(true);
        nextVehicle->SetStatus(VehicleStatus::CROSSING);
        lane->SetCurrentCrossing(nextVehicle);
        lane->RemoveVehicle(nextVehicle);
    }
}

void Intersection::PrintState(int currentTime) const {
    std::cout << "Intersection " << id << " - Active Lane: " << activeLane << std::endl;
    for (int i = 0; i < numLanes; i++) {
        std::cout << "  Lane " << i << ": ";
        lanes[i].PrintState();
    }
}

bool Intersection::HasWaitingVehicles() const {
    for (int i = 0; i < numLanes; i++) {
        if (lanes[i].HasWaitingVehicles()) return true;
    }
    return false;
}

bool Intersection::HasCrossingVehicles() const {
    for (int i = 0; i < numLanes; i++) {
        if (lanes[i].GetCurrentCrossing() != nullptr) return true;
    }
    return false;
}

