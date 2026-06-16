#ifndef LANE_H
#define LANE_H

#include "Queue.h"
#include "PriorityQueue.h"
#include "Vehicle.h"

inline bool CompareEVPriority(Vehicle* const& a, Vehicle* const& b) {
    if (a == nullptr || b == nullptr) return false;
    if (a->GetPriorityScore() != b->GetPriorityScore()) {
        return a->GetPriorityScore() > b->GetPriorityScore();
    }
    return a->GetArrivalTime() < b->GetArrivalTime();
}

class Lane {
private:
    Queue<Vehicle*> ptQueue;
    Queue<Vehicle*> ncQueue;
    Queue<Vehicle*> fvQueue;
    PriorityQueue<Vehicle*> evQueue;
    Vehicle* currentCrossing;
    bool isBlocked;
    bool isLocked;
    int lockTime;
    int blockEndTime;

public:
    Lane();
    ~Lane();

    bool AddVehicle(Vehicle* vehicle);
    bool RemoveVehicle(Vehicle* vehicle);
    bool HasWaitingVehicles() const;
    Vehicle* SelectNextVehicle();

    Vehicle* GetCurrentCrossing() const;
    void SetCurrentCrossing(Vehicle* vehicle);
    bool IsBlocked() const;
    void SetBlocked(bool blocked);
    void SetBlocked(bool blocked, int endTime);
    int GetBlockEndTime() const;
    bool IsLocked() const;
    void SetLocked(bool locked, int time);
    int GetLockTime() const;
    int GetPTCount() const;
    int GetNCCount() const;
    int GetFVCount() const;
    int GetEVCount() const;
    int GetTotalWaitingCount() const;
    void PrintState() const;
    
    void GetEVQueueIDs(int* ids, int& count) const;
    void GetPTQueueIDs(int* ids, int& count) const;
    void GetNCQueueIDs(int* ids, int& count) const;
    void GetFVQueueIDs(int* ids, int& count) const;
};

#endif

