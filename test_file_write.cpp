// MINIMAL GUARANTEED-WORKING FILE WRITE TEST
// Compile and run this to verify file writing works in your environment

#include <fstream>
#include <iostream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

int main() {
    // Get current directory
    std::string currentDir = "";
#ifdef _WIN32
    char dir[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, dir)) {
        currentDir = dir;
    }
#else
    char dir[PATH_MAX];
    if (getcwd(dir, PATH_MAX)) {
        currentDir = dir;
    }
#endif
    
    std::cout << "Current working directory: " << currentDir << std::endl;
    
    // Try to write a test file
    const char* testFile = "test_output.txt";
    std::cout << "Attempting to write: " << testFile << std::endl;
    std::cout << "Full path: " << currentDir << "\\" << testFile << std::endl;
    
    std::ofstream file(testFile, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open file!" << std::endl;
        return 1;
    }
    
    file << "This is a test file." << std::endl;
    file << "If you see this, file writing works!" << std::endl;
    file << "Current directory: " << currentDir << std::endl;
    
    file.flush();
    file.close();
    
    if (file.fail()) {
        std::cerr << "ERROR: Failed to write file!" << std::endl;
        return 1;
    }
    
    std::cout << "SUCCESS: File written successfully!" << std::endl;
    std::cout << "Check for: " << currentDir << "\\" << testFile << std::endl;
    
    return 0;
}

