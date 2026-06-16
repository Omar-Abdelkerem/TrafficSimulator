#ifndef UI_H
#define UI_H

class TrafficControlCenter;
class Intersection;
class Lane;

class UI {
public:
    enum SimulationMode {
        INTERACTIVE = 0,
        STEP_BY_STEP = 1,
        SILENT = 2
    };

    UI();
    ~UI();
    UI(const UI&) = delete;
    UI& operator=(const UI&) = delete;

    void Run(const char* inputFile, const char* outputFile);
    SimulationMode ChooseSimulationMode();
    void PrintTimestepHeader(int time);
    void PrintIntersectionState(const Intersection* intersection, int time);
    void PrintQueues(const Lane* lane);
    void PrintFinalMessage(const TrafficControlCenter* tcc);
    void WaitForInput();
    void PrintSeparator();
    
private:
    TrafficControlCenter* tcc;
};

#endif

