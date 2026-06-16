#include "UI.h"
#include <iostream>

int main(int argc, char* argv[]) {
    const char* inputFile = "input_complex.txt";
    const char* outputFile = "output.txt";

    if (argc >= 2) inputFile = argv[1];
    if (argc >= 3) outputFile = argv[2];

    UI ui;
    ui.Run(inputFile, outputFile);

    return 0;
}

