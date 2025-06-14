#ifndef TINYWSPRENCODE_H
#define TINYWSPRENCODE_H

#include <stdint.h>

class tinyWSPREncode
{
    public:

    // Generates full symbol buffer and calls all the other functions
    void encode(const char * callsign, const char * locator, const int8_t dBm, uint8_t * outputSymbolBuffer);

    // Convert GPS locations to maidenhead grid locator
    void GPSToGrid(double latitude, double longitude, uint8_t * grid);

    private:
    // "Source Coding" in PDF
    uint64_t message_setup(const char * callsign, const char * locator, int8_t dBm);

    // "Bit Packing" in PDF
    void pack_bits(uint64_t inputMessage, uint8_t *outputBuffer);

     // "Convolutional Encoding" in PDF
    void convolve(uint8_t *inputBuffer, uint8_t * outputBuffer);
    
    // "Interleaving" in PDF
    void interleave(uint8_t *inputBuffer, uint8_t *outputBuffer);

    // "Merge With Sync Vector" in PDF
    void merge_sync_vector(uint8_t * inputBuffer, uint8_t * outputSymbolBuffer);

    uint8_t ascii_to_wspr(char c);

   
    // Sync vector for transmission
    static constexpr uint8_t SYNC_VECTOR[21] = {
        0b11000000, 0b10001110, 0b00100101, 0b11100000, 0b00100101, 0b00000010, 0b11001101, 
        0b00011010,0b00011010, 0b10101001, 0b00101100, 0b01101010, 0b00100000, 0b10010011, 
        0b10110011, 0b01000111, 0b00000101, 0b00110000, 0b00011010, 0b11000110, 0b00000000

    };
   
    uint8_t bufferElement;
    uint8_t bufferElementBit;


};

#endif
