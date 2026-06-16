#include "Event.h"

Event::Event() 
    : time(0), eventType(' '), vehicleID(0), intersectionID(0), 
      laneID(0), parameter1(0), parameter2(0), vehicleSubtype(' ') {
}

Event::Event(int t, char type, char subtype, int vID, int interID, int lID, int param1, int param2)
    : time(t), eventType(type), vehicleID(vID), intersectionID(interID),
      laneID(lID), parameter1(param1), parameter2(param2), vehicleSubtype(subtype) {
}

Event::Event(int t, char type, int vID)
    : time(t), eventType(type), vehicleID(vID), intersectionID(0),
      laneID(0), parameter1(0), parameter2(0), vehicleSubtype(' ') {
}

Event::Event(int t, char type, int interID, int lID, int duration)
    : time(t), eventType(type), vehicleID(0), intersectionID(interID),
      laneID(lID), parameter1(duration), parameter2(0), vehicleSubtype(' ') {
}

Event::~Event() {
}

int Event::GetTime() const { return time; }
char Event::GetType() const { return eventType; }
char Event::GetVehicleSubtype() const { return vehicleSubtype; }
int Event::GetVehicleID() const { return vehicleID; }
int Event::GetIntersectionID() const { return intersectionID; }
int Event::GetLaneID() const { return laneID; }
int Event::GetParameter1() const { return parameter1; }
int Event::GetParameter2() const { return parameter2; }

void Event::SetTime(int t) { time = t; }
void Event::SetType(char type) { eventType = type; }
void Event::SetVehicleID(int id) { vehicleID = id; }
void Event::SetIntersectionID(int id) { intersectionID = id; }
void Event::SetLaneID(int id) { laneID = id; }
void Event::SetParameter1(int param) { parameter1 = param; }
void Event::SetParameter2(int param) { parameter2 = param; }
void Event::SetVehicleSubtype(char subtype) { vehicleSubtype = subtype; }

void Event::Print() const {
    std::cout << "Event: Type=" << eventType;
    if (eventType == 'A' && vehicleSubtype != ' ') {
        std::cout << "(" << vehicleSubtype << ")";
    }
    std::cout << ", Time=" << time;
    if (eventType == 'A' || eventType == 'X' || eventType == 'P') {
        std::cout << ", VehicleID=" << vehicleID;
    }
    if (eventType == 'A' || eventType == 'R') {
        std::cout << ", Intersection=" << intersectionID << ", Lane=" << laneID;
    }
    std::cout << std::endl;
}

