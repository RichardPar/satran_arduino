/*************************************************************
* SATRAN firmware v1.0
* License Creative Commons BY-NC-SA 2.0 (non-commercial)
* Copyright 2021 Danaco, Daniel Nikolajsen <satran@danaco.se>
*************************************************************/

Firmware code for the Satran Mk1; ESP8266 NodeMCU v2 microcontroller

The firmware and control board can also be used to control any
project requiring 2pcs stepper motors or 2 simple on/off inputs

Danaco gives no warranty of any sort for this code, if you have
any questions regarding its use please use the forum at www.satran.io

This code be written over USB directly to the NodeMCU microcontroller,
using the free Arduino IDE software

IMPORTANT! First time you compile the firmware:
1. Uncomment the EEPROM clearing function in the beginning of setup()
2. Compile and write to the board
3. The board reboots, clearing space in the eeprom
4. Comment out or delete the EEPROM function again
5. Compile and write to the board
Finished!
