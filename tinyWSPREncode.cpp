#include "tinyWSPREncode.h"
#include <stdio.h>
#include <stdint.h>


#include <string.h>

void tinyWSPREncode::encode(const char * callsign, const char * locator, const int8_t dBm, uint8_t * outputSymbolBuffer) {

    
    // Convert text call sign, locator, and power to 50 bits
    uint64_t codedMessage = message_setup(callsign, locator, dBm);

    // Array to store packed coded message
    uint8_t packedMessage[11] = {}; // Must be 0
    pack_bits(codedMessage, packedMessage);

    // Convolve to add error correction and expand from 50 source data bits to 162 output bits
    uint8_t convolvedMessage[21] = {};
    convolve(packedMessage, convolvedMessage);

    // Move data location around in message to account for random interference during transmission
    uint8_t interleavedMessage[21] = {};
    interleave(convolvedMessage, interleavedMessage);

    // Merge with sync vector and output to the output symbol buffer.
    // TODO: Create sync array
    merge_sync_vector(interleavedMessage, outputSymbolBuffer);


}

// Converts ASCII character to WSPR base 37 symbol table. 
// "0-9" is 0-9, "A-Z" is 10-35, " " is 36
uint8_t tinyWSPREncode::ascii_to_wspr(char c) {

    if (c >= '0' && c <= '9') { return c - 48; }
    if (c >= 'A' && c <= 'Z') { return c - 55; }
    if (c == ' ') { return 36; }

}
uint64_t tinyWSPREncode::message_setup(const char * callsign, const char * locator, int8_t dBm) {



    uint32_t n;
    uint32_t m;

    // Encode the callsign to a single int n as described in the WSPR protocol
    // n is a 32 bit in but actual data is only 28 bits
    n = ascii_to_wspr(callsign[0]);
    n = (n * 36) + ascii_to_wspr(callsign[1]);
    n = (n * 10) + ascii_to_wspr(callsign[2]);
    n = (n * 27) + (ascii_to_wspr(callsign[3]) - 10);
    n = (n * 27) + (ascii_to_wspr(callsign[4]) - 10);
    n = (n * 27) + (ascii_to_wspr(callsign[5]) - 10);

    // Encode the locator and power (dBm) to a single int m as described in the WSPR protocol
    // n is a 32 bit in but actual data is only 22 bits
    m = ((179 - (10 * (ascii_to_wspr(locator[0])-10)) - ascii_to_wspr(locator[2])) * 180) + (10 * (ascii_to_wspr(locator[1])-10)) + ascii_to_wspr(locator[3]);
    m = (m * 128) + dBm + 64;

    // Cast n and m to 64 bit ints, shift them so the int is the 28 bits of n followed by 22 bits of m appended by 0s 
    return ((static_cast<uint64_t>(n) << 36) | (static_cast<uint64_t>(m) << 14));
}


// Cast the 64-bit input int into an array of 11 8-bit ints
void tinyWSPREncode::pack_bits(uint64_t inputMessage, uint8_t *outputBuffer) {

    // Set outputBuffer array to all 0s so the end of it that inputMessage int doesn't fil are 0s
    memset(outputBuffer, 0, 11);

    for (int n = 0; n < 8; n++) {

    // Shift 64 bit input message right (8 bits * output array position) so desired group of 8 bits is at bits 0-7 of 64-bit int
    // Cast that 64-bit int to the 8-bit int array position. Bits 0-7 of 64-bit int go to bits 0-7 of new 8-bit int array position.
    // When the outputBuffer position (n) is advanced the 64-bit message int gets shifted correct amount based on position n.
    outputBuffer[n] = static_cast<uint8_t>(inputMessage >> (64-((n+1)*8)));

    }
}

void tinyWSPREncode::convolve(uint8_t *inputBuffer, uint8_t * outputBuffer) {

    uint32_t reg = 0;
    uint8_t result;
    uint32_t masked = 0;

    // Loop through 81 input bits (full input is an array of 11 8-bit ints so 88 bits total but we only need 81)
    for(int i = 0; i < 81; i++) {

        // we want to do operations on each bit of input array starting from MSB to LSB of each array element. 
        // these variables tell us which input array element and which bit of that array to look at for a given bit of message that we want
        // bufferElementBit is 7 - because we are shifting that element right by that number to get the right bit in the LSB position in the next step
        bufferElement = i / 8;
        bufferElementBit = 7 - (i % 8);

        
        // Use previously gotten element variables to shift desiresd bit into LSB bit position. AND with 1 to get only that bit of interest. 
        // Shift the rolling register left 1 and OR that desired bit into the LSB position of reg
        reg = (reg << 1) | static_cast<uint32_t>((inputBuffer[bufferElement] >> bufferElementBit) & 1);
        

        // Mask the reg with the first feedback tap
        masked = reg & 0xF2D05351;
        
        // Calculate the parity of the first feedback tap mask
        result = 0;
        while (masked) {
            result ^= (masked & 1);
            masked >>= 1;
        }

        // Calculate the array element and element bit of output array (double the length of input array bc there are two output bits for each input bit)
        bufferElement = (i*2) / 8;
        bufferElementBit = 7 - ((i*2) % 8);

        // Append the first feedback tap parity bit to the output array
        outputBuffer[bufferElement] |= result << bufferElementBit;
        
        // Mask the reg with the second feedback tap
        masked = reg & 0xE4613C47;

        // Calculate the parity of the second feedback tap mask
        result = 0;
        while (masked) {
            result ^= (masked & 1);
            masked >>= 1;
        }

        // Append the second feedback tap parity to the output array
        outputBuffer[bufferElement] |= result << (bufferElementBit - 1);

    }

}


void tinyWSPREncode::interleave(uint8_t *inputBuffer, uint8_t *outputBuffer) {

    
    uint8_t bit;
    int p = 0;

    uint8_t j; // Bit reversed i

    
    for (int i = 0; i < 256; ++i) {

        // Bit reverse i. Output is j
        j = 0;
        for (int x = 0; x < 8; ++x) {
            j |= ((i >> x) & 1) << (7 - x);
        }

        

        if (j < 162) {

            // Get inputBuffer[p] bufferElement and bufferElementBit
            bufferElement = p / 8;
            bufferElementBit = 7 - (p % 8);

            // Save inputBuffer[p] to bit variable
            bit = (inputBuffer[bufferElement] >> bufferElementBit) & 1;

            // Get outputBuffer[j] bufferElement and bufferElementBit
            bufferElement = j / 8;
            bufferElementBit = 7 - (j % 8);

            // Save inputBuffer[p] (stored in bit var) to outputBuffer[j]
            outputBuffer[bufferElement] |= (bit << bufferElementBit);

            p++;
            
        }

        

    }

}

void tinyWSPREncode::merge_sync_vector(uint8_t * inputBuffer, uint8_t * outputSymbolBuffer) {
    
    uint8_t inputBit;
    uint8_t syncBit;


    for (int n = 0; n < 162; n++) {

        // Find which array element and shift value is needed to get a certain bit number
        bufferElement = n / 8;
        bufferElementBit = 7 - (n % 8);

        // Store that position's bit value for the input array and sync array in variables
        inputBit = (inputBuffer[bufferElement] >> bufferElementBit) & 1;
        syncBit = (SYNC_VECTOR[bufferElement] >> bufferElementBit) & 1;

        // Set the output symbol buffer at that position based on this equation to get the actual symbol value
        // The symbol can be 0-3
        outputSymbolBuffer[n] = (syncBit + (2 * inputBit));
       

    }

}

void tinyWSPREncode::GPSToGrid(double latitude, double longitude, uint8_t * grid) {

  longitude += 180.0;
  latitude  += 90.0;

  grid[0] = 'A' + int(longitude / 20);
  grid[1] = 'A' + int(latitude / 10);
  grid[2] = '0' + ((int(longitude) % 20) / 2);
  grid[3] = '0' + (int(latitude) % 10);
  grid[4] = '\0';

}