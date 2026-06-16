#include "Lane.h"
#include <iostream>

Lane::Lane() 
    : evQueue(CompareEVPriority), currentCrossing(nullptr), 
      isBlocked(false), isLocked(false), lockTime(0), blockEndTime(-1) {
}

Lane::~Lane() {
}

bool Lane::AddVehicle(Vehicle* vehicle) {
    if (vehicle == nullptr) return false;
    char type = vehicle->GetTypeChar();
    switch (type) {
        case 'P': ptQueue.Enqueue(vehicle); return true;
        case 'N': ncQueue.Enqueue(vehicle); return true;
        case 'F': fvQueue.Enqueue(vehicle); return true;
        case 'E': evQueue.Enqueue(vehicle); return true;
        default: return false;
    }
}

bool Lane::RemoveVehicle(Vehicle* vehicle) {
    if (vehicle == nullptr) return false;
    char type = vehicle->GetTypeChar();
    switch (type) {
        case 'P': return ptQueue.Remove(vehicle);
        case 'N': return ncQueue.Remove(vehicle);
        case 'F': return fvQueue.Remove(vehicle);
        case 'E': return evQueue.Remove(vehicle);
        default: return false;
    }
}

bool Lane::HasWaitingVehicles() const {
    return !ptQueue.IsEmpty() || !ncQueue.IsEmpty() || 
           !fvQueue.IsEmpty() || !evQueue.IsEmpty();
}

Vehicle* Lane::SelectNextVehicle() {
    if (isBlocked || isLocked) return nullptr;

    while (!evQueue.IsEmpty()) {
        Vehicle* vehicle = evQueue.Peek();
        if (vehicle != nullptr && !vehicle->IsCanceled()) return vehicle;
        evQueue.Dequeue();
    }

    while (!ptQueue.IsEmpty()) {
        Vehicle* vehicle = ptQueue.Peek();
        if (vehicle != nullptr && !vehicle->IsCanceled()) return vehicle;
        ptQueue.Dequeue();
    }

    while (!ncQueue.IsEmpty()) {
        Vehicle* vehicle = ncQueue.Peek();
        if (vehicle != nullptr && !vehicle->IsCanceled()) return vehicle;
        ncQueue.Dequeue();
    }

    while (!fvQueue.IsEmpty()) {
        Vehicle* vehicle = fvQueue.Peek();
        if (vehicle != nullptr && !vehicle->IsCanceled()) return vehicle;
        fvQueue.Dequeue();
    }

    return nullptr;
}

Vehicle* Lane::GetCurrentCrossing() const { return currentCrossing; }
void Lane::SetCurrentCrossing(Vehicle* vehicle) { currentCrossing = vehicle; }
bool Lane::IsBlocked() const { return isBlocked; }
void Lane::SetBlocked(bool blocked) { 
    isBlocked = blocked; 
    if (!blocked) blockEndTime = -1;
}
void Lane::SetBlocked(bool blocked, int endTime) { 
    isBlocked = blocked; 
    blockEndTime = endTime;
}
int Lane::GetBlockEndTime() const { return blockEndTime; }
bool Lane::IsLocked() const { return isLocked; }
void Lane::SetLocked(bool locked, int time) {
    isLocked = locked;
    if (locked) lockTime = time;
}
int Lane::GetLockTime() const { return lockTime; }
int Lane::GetPTCount() const { return ptQueue.Count(); }
int Lane::GetNCCount() const { return ncQueue.Count(); }
int Lane::GetFVCount() const { return fvQueue.Count(); }
int Lane::GetEVCount() const { return evQueue.Count(); }
int Lane::GetTotalWaitingCount() const {
    return ptQueue.Count() + ncQueue.Count() + fvQueue.Count() + evQueue.Count();
}

void Lane::PrintState() const {
    std::cout << "  PT: " << ptQueue.Count() << ", NC: " << ncQueue.Count()
              << ", FV: " << fvQueue.Count() << ", EV: " << evQueue.Count() << std::endl;
}

void Lane::GetEVQueueIDs(int* ids, int& count) const {
    count = 0;
    Node<Vehicle*>* current = evQueue.GetHead();
    while (current != nullptr && count < 100) {
        if (current->data != nullptr && !current->data->IsCanceled()) {
            ids[count++] = current->data->GetID();
        }
        current = current->next;
    }
}

void Lane::GetPTQueueIDs(int* ids, int& count) const {
    count = 0;
    Node<Vehicle*>* current = ptQueue.GetHead();
    while (current != nullptr && count < 100) {
        if (current->data != nullptr && !current->data->IsCanceled()) {
            ids[count++] = current->data->GetID();
        }
        current = current->next;
    }
}

void Lane::GetNCQueueIDs(int* ids, int& count) const {
    count = 0;
    Node<Vehicle*>* current = ncQueue.GetHead();
    while (current != nullptr && count < 100) {
        if (current->data != nullptr && !current->data->IsCanceled()) {
            ids[count++] = current->data->GetID();
        }
        current = current->next;
    }
}

void Lane::GetFVQueueIDs(int* ids, int& count) const {
    count = 0;
    Node<Vehicle*>* current = fvQueue.GetHead();
    while (current != nullptr && count < 100) {
        if (current->data != nullptr && !current->data->IsCanceled()) {
            ids[count++] = current->data->GetID();
        }
        current = current->next;
    }
}

