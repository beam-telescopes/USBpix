USBpix FPGA firmware for FE-I4

11.06.2010
Newest Releases: 
- USBpix_V06.bit

NOTE: Due to a changes in the µC-frequency both work ONLY with the µC-firmware usbpix_FW_13.bix (or usbpix_FW_13.hex to flash EEPROM)...

Functionality:

1. Storage of 8b10b encoded and raw data transmitted with 40 or 160 Mbit/s. Idle state detection, SOF, EOF detection and storage of all data between SOF and EOF in the SRAM. 
After filling the SRAM and clicking "ReadSRAM" Button, a file "dataRB.dat" is written wich lists the received data words in binary code. Note that this is just for debugging 
and writing this file will not be done automatically later...
New: Word errro rate test implemented:
Automatic filling of SRAM, comparison between read and expected data, calculating error rate, clearing SRAM, restart reading...

2. Sending Bitstreams to FE. Read and Write registers in emulator. 

3. Sending LV1 signals using CMOS signal for external pad and CMD with variable delay and length. Sending command "Strobe" in variable frequencies and numbers.

4. Use every 20bit pattern on line do0 to check and set synchronization phase.


All Signals are available on debug-connector P9:
Pin  2: XCK          			--> 40 MHz clock
Pin  4: DI          		  --> data to FE in CMD - mode
Pin  6: INMUX[0]     
Pin  8: INMUX[1]     
Pin 10: INMUX[2]     
Pin 12: INMUX[3]
Pin 14: DO0           		--> data coming from FE-I4 - emulator (ENCODED!!!)
Pin 16: INMUX_SELECT[0]   --> select between bypass and CMD mode

GND: Pin 1, 3, 5, 7, 9, 11, 13, 15 

LEDs:
LED 1: flashing if firmware download was succesfull
LED 2: off in idle state, on if SRAM full. Writing SRAM is blocked when on. Press "restart data take" to escape this system-state  ;-)
LED 3: on if Synchronization check (BER test) is busy
LED 4: on if system is in TOT-histogramming mode (not yet implemented)
LED 5: on if system is in hit-histogramming mode (not yet implemented) 

TX0: GND
TX1: DO0


















































Release V0.0

Malte Backhaus, University of Bonn

Feb. 01. 2010

Note:

USBpixI4_V00.bit is a test firmware with limited functionality. It is meant to be connected to the FE-I4 emulator Version 0.0 developed by Laura Gonella.

SET ALL SOLDER JUMPERS ON THE BOTTOM SIDE OF THE USBPIX FE-3 ADAPTER CARD !!!

Functionality:

1. Storage of 8b10b encoded data transmitted with 40 MHz. Idle state detection, SOF, EOF detection and storage of all data between SOF and EOF in the SRAM. Due to timing some bytes of the SRAM are not read correctly. After filling the SRAM and clicking "ReadSRAM" Button, a file "dataRB.dat" is written wich lists the received data words in binary code. Note that this is just for debugging and writing this file will not be done automatically later.

2. Sending Bitstreams to FE (NOT SUPPORTED BY EMULATOR YET). Configuring FE with dummy data using CMD and bypass mode. 

3. Sending LV1 signals using CMOS signal for external pad and CMD with variable delay and length. Sending command "Strobe" in variable frequencies and numbers.


All Signals are available on debug-connector P9:
Pin  1: XCK          --> 40 MHz clock
Pin  2: DI           --> data to FE in CMD - mode
Pin  3: INMUX[0]     
Pin  4: INMUX[1]     
Pin  5: INMUX[2]     
Pin  6: INMUX[3]
Pin  7: DO0           --> data coming from FE-I4 - emulator (ENCODED!!!)
Pin  8: DO0-intern    --> data from FE-I4  - emulator decoded!!!
Pin  9: CMOS LV1
Pin 10: INMUXselect[0]--> set to 1 if system is in bypass-mode  

Pin 15: GND  

LEDs:
LED 1: flashing if firmware download was succesfull
LED 2: off in idle state, on if SRAM full. Writing SRAM is blocked when on. Press "restart data take" to escape this system-state  ;-)
LED 3: on if µC busy with scan (not yet implemented)
LED 4: on if system is in TOT-histogramming mode (not yet implemented)
LED 5: on if system is in hit-histogramming mode (not yet implemented) 

TX0: XCK
TX1: DO0


