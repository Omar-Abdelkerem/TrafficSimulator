#ifndef EVENT_H
#define EVENT_H

#include <iostream>

class Event {
private:
    int time;
    char eventType;
    int vehicleID;
    int intersectionID;
    int laneID;
    int parameter1;
    int parameter2;
    char vehicleSubtype;

public:
    Event();
    Event(int t, char type, char subtype, int vID, int interID, int lID, int param1, int param2 = 0);
    Event(int t, char type, int vID);
    Event(int t, char type, int interID, int lID, int duration);
    ~Event();

    int GetTime() const;
    char GetType() const;
    char GetVehicleSubtype() const;
    int GetVehicleID() const;
    int GetIntersectionID() const;
    int GetLaneID() const;
    int GetParameter1() const;
    int GetParameter2() const;

    void SetTime(int t);
    void SetType(char type);
    void SetVehicleID(int id);
    void SetIntersectionID(int id);
    void SetLaneID(int id);
    void SetParameter1(int param);
    void SetParameter2(int param);
    void SetVehicleSubtype(char subtype);

    void Print() const;
    
    bool operator==(const Event& other) const { return time == other.time && vehicleID == other.vehicleID; }
};

#endif

