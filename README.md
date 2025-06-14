# WSPR-Encoder
A lightweight, microcontroller-friendly, C++ WSPR Encoder designed for use in hardware-constrained embedded enviroments. 

## Purpose and Usage
This library was originally designed for use in an ATTiny1616-based Picoballoon tracker. Because of memory constraints, all non-essential functions and source formatting has not been included. 

Because of the lack of source formatting (TO-DO), the call sign, locator, and dBm fields must be inputed already fitting the WSPR structure. The callsign input must be 6 characters. 6 character callsigns can be inputted directly. The third character of the callsign input must be a number. To accomidate callsigns with a letter followed immedietly by a number, a space should be appended to the beginning of the callsign. Short callsigns also need to be padded to 6 characters by appending spaces to the end. 
