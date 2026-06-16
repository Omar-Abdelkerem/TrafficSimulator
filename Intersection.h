#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "Lane.h"

class Intersection {
private:
    int id;
    Lane* lanes;
    int numLanes;
    int activeLane;
    int switchingCost;
    bool isClosed;
    int switchCounter;
    int closureEndTime;

public:
    Intersection(int intersectionID, int numberOfLanes, int switchCost);
    ~Intersection();
    Intersection(const Intersection&) = delete;
    Intersection& operator=(const Intersection&) = delete;

    int GetID() const;
    int GetNumLanes() const;
    int GetActiveLane() const;
    Lane* GetLane(int laneIndex) const;
    bool IsClosed() const;
    void SetClosed(bool closed);
    void SetClosed(bool closed, int endTime);
    int GetClosureEndTime() const;

    int ScheduleLane(int currentTime);
    bool SwitchLane(int newLaneIndex, int currentTime);
    void HandleCrossing(int currentTime);
    void PrintState(int currentTime) const;
    bool HasWaitingVehicles() const;
    bool HasCrossingVehicles() const;
    int GetSwitchCount() const { return switchCounter; }
};

#endif

