#include <iostream>
#include <bitset>
#include "tinyWSPREncode.h"

tinyWSPREncode encoder;

char CALLSIGN[] = "AB1CDE";
char GRID[] = "AB12";
int dBm = 30;

// Function to print the symbol buffer
void printSymbolBuffer(uint8_t* buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        std::cout << static_cast<int>(buffer[i]);
        if (i < length - 1) {
            std::cout << ",";
        }
    }
    std::cout << std::endl;
}


int main() {

    // Array to store the output WSPR symbol buffer
    uint8_t outputSymbolBuffer[162] = {0};

    // Encode to WSPR symbol buffer
    encoder.encode(CALLSIGN, GRID, dBm, outputSymbolBuffer);

    // Print out the buffer
    printSymbolBuffer(outputSymbolBuffer, 162);
    return 0;
}