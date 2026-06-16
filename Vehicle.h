#ifndef VEHICLE_H
#define VEHICLE_H

#include <iostream>

enum class VehicleStatus {
    WAITING, CROSSING, COMPLETED, CANCELED
};

class Vehicle {
protected:
    int id;
    int arrivalTime;
    int waitingTime;
    int crossingDuration;
    int completionTime;
    int intersectionID;
    int laneID;
    VehicleStatus status;
    int originalCrossingDuration;

public:
    Vehicle();
    Vehicle(int vehicleID, int arrTime, int crossDur, int interID, int lID);
    virtual ~Vehicle();

    int GetID() const;
    virtual char GetTypeChar() const = 0;
    int GetArrivalTime() const;
    int GetWaitingTime() const;
    int GetCrossingDuration() const;
    int GetOriginalCrossingDuration() const;
    int GetCompletionTime() const;
    int GetIntersectionID() const;
    int GetLaneID() const;
    VehicleStatus GetStatus() const;
    bool IsCanceled() const;
    bool IsCrossing() const;
    bool IsWaiting() const;
    bool IsCompleted() const;
    virtual int GetPriorityScore() const { return 0; }

    void SetIntersectionID(int id);
    void SetLaneID(int id);
    void SetCanceled(bool canceled);
    void SetCrossing(bool crossing);
    void SetCompletionTime(int time);
    void SetStatus(VehicleStatus newStatus);
    void SetCrossingDuration(int duration);
    void DecrementCrossingDuration();
    void UpdateWaitingTime(int currentTime);
    virtual void Print() const;
    
    bool operator==(const Vehicle& other) const { return id == other.id; }
};

class EV : public Vehicle {
private:
    int priorityScore;

public:
    EV(int vehicleID, int arrTime, int crossDur, int interID, int lID, int priority);
    virtual ~EV();
    char GetTypeChar() const override { return 'E'; }
    int GetPriorityScore() const override;
    void SetPriorityScore(int score);
    void Print() const override;
};

class PT : public Vehicle {
public:
    PT(int vehicleID, int arrTime, int crossDur, int interID, int lID);
    virtual ~PT();
    char GetTypeChar() const override { return 'P'; }
    void Print() const override;
};

class NC : public Vehicle {
public:
    NC(int vehicleID, int arrTime, int crossDur, int interID, int lID);
    virtual ~NC();
    char GetTypeChar() const override { return 'N'; }
    void Print() const override;
};

class FV : public Vehicle {
public:
    FV(int vehicleID, int arrTime, int crossDur, int interID, int lID);
    virtual ~FV();
    char GetTypeChar() const override { return 'F'; }
    void Print() const override;
};

#endif

