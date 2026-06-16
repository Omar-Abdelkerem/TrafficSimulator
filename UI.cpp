#include "UI.h"
#include "TrafficControlCenter.h"
#include "Intersection.h"
#include "Lane.h"
#include "Vehicle.h"
#include <iostream>
#include <limits>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

void SleepMilliseconds(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

UI::UI() : tcc(nullptr) {
}

UI::~UI() {
    if (tcc != nullptr) {
        delete tcc;
    }
}

void UI::Run(const char* inputFile, const char* outputFile) {
    SimulationMode mode = ChooseSimulationMode();
    
    tcc = new TrafficControlCenter();
    
    int modeInt = (mode == INTERACTIVE) ? 0 : (mode == STEP_BY_STEP) ? 1 : 2;
    if (!tcc->RunSimulation(inputFile, outputFile, this, modeInt)) {
        std::cerr << "Error: Simulation failed!" << std::endl;
        return;
    }
    
    PrintFinalMessage(tcc);

    try {
        std::filesystem::path outPath(outputFile);
        std::error_code ec;
        std::filesystem::path absPath = std::filesystem::absolute(outPath, ec);
        if (ec) {
            std::cout << "Output written to: " << outPath.string() << std::endl;
        } else {
            std::cout << "Output written to: " << absPath.string() << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cout << "Output written to: " << outputFile << std::endl;
    }
    
    if (mode == INTERACTIVE) {
        WaitForInput();
    } else if (mode == STEP_BY_STEP) {
        SleepMilliseconds(2000);
    }
    
    std::cout << "Simulation completed successfully!" << std::endl;
}

UI::SimulationMode UI::ChooseSimulationMode() {
    std::cout << "=== Traffic Control Center Simulation ===" << std::endl;
    std::cout << "Choose simulation mode:" << std::endl;
    std::cout << "1. Interactive (press key to advance)" << std::endl;
    std::cout << "2. Step-by-Step (automatic with delay)" << std::endl;
    std::cout << "3. Silent (no output until end)" << std::endl;
    std::cout << "Enter choice (1-3): ";

    int choice;
    std::cin >> choice;
    while (choice < 1 || choice > 3) {
        std::cout << "Invalid choice. Please enter 1, 2, or 3: ";
        std::cin >> choice;
    }

    switch (choice) {
        case 1: return INTERACTIVE;
        case 2: return STEP_BY_STEP;
        case 3:
        default: return SILENT;
    }
}

void UI::PrintTimestepHeader(int time) {
    std::cout << std::endl;
    std::cout << "Current Timestep: " << time << std::endl;
}

void UI::PrintIntersectionState(const Intersection* intersection, int time) {
    if (intersection == nullptr) return;
    
    std::cout << "=============== INTERSECTION #" << intersection->GetID() << " ===============" << std::endl;
    
    Lane* activeLane = intersection->GetLane(intersection->GetActiveLane());
    if (activeLane != nullptr) {
        int evIDs[100], ptIDs[100], ncIDs[100], fvIDs[100];
        int evCount = 0, ptCount = 0, ncCount = 0, fvCount = 0;
        
        activeLane->GetEVQueueIDs(evIDs, evCount);
        activeLane->GetPTQueueIDs(ptIDs, ptCount);
        activeLane->GetNCQueueIDs(ncIDs, ncCount);
        activeLane->GetFVQueueIDs(fvIDs, fvCount);
        
        std::cout << "EV queue: ";
        if (evCount == 0) {
            std::cout << "(empty)";
        } else {
            for (int j = 0; j < evCount; j++) {
                std::cout << evIDs[j];
                if (j < evCount - 1) std::cout << ", ";
            }
        }
        std::cout << std::endl;
        
        std::cout << "PT queue: ";
        if (ptCount == 0) {
            std::cout << "(empty)";
        } else {
            for (int j = 0; j < ptCount; j++) {
                std::cout << ptIDs[j];
                if (j < ptCount - 1) std::cout << ", ";
            }
        }
        std::cout << std::endl;
        
        std::cout << "NC queue: ";
        if (ncCount == 0) {
            std::cout << "(empty)";
        } else {
            for (int j = 0; j < ncCount; j++) {
                std::cout << ncIDs[j];
                if (j < ncCount - 1) std::cout << ", ";
            }
        }
        std::cout << std::endl;
        
        std::cout << "FV queue: ";
        if (fvCount == 0) {
            std::cout << "(empty)";
        } else {
            for (int j = 0; j < fvCount; j++) {
                std::cout << fvIDs[j];
                if (j < fvCount - 1) std::cout << ", ";
            }
        }
        std::cout << std::endl;
        
        std::cout << "Active Green Lane: LN" << (intersection->GetActiveLane() + 1) << std::endl;
        
        Vehicle* crossing = activeLane->GetCurrentCrossing();
        if (crossing != nullptr) {
            std::cout << "Vehicles Crossing: " << crossing->GetID() 
                      << " (XD remaining = " << crossing->GetCrossingDuration() << ")" << std::endl;
        } else {
            std::cout << "Vehicles Crossing: (none)" << std::endl;
        }
    }
    
    std::cout << "----------------------------------------------" << std::endl;
}

void UI::PrintQueues(const Lane* lane) {
    if (lane == nullptr) return;
    std::cout << "  PT: " << lane->GetPTCount() << ", NC: " << lane->GetNCCount()
              << ", FV: " << lane->GetFVCount() << ", EV: " << lane->GetEVCount() << std::endl;
}

void UI::PrintFinalMessage(const TrafficControlCenter* tcc) {
    if (tcc == nullptr) return;
    std::cout << std::endl;
    PrintSeparator();
    std::cout << "Simulation Complete!" << std::endl;
    PrintSeparator();
    std::cout << "Final Time: " << tcc->GetCurrentTimeValue() << std::endl;
    std::cout << "Check output.txt for detailed statistics." << std::endl;
    PrintSeparator();
}

void UI::WaitForInput() {
    std::cout << "Press Enter to display next timestep...";
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    std::cin.get();
}

void UI::PrintSeparator() {
    std::cout << "----------------------------------------" << std::endl;
}

