# Electrical Subteam for Stormwater Runoff Device

This is the GitHub to maintain PCB (Printed Circuit Board) files for the EPA Rainworks Project. This 
board is relatively simple:
- 5 sensors that all connect with simply a VDD, GND, and INPUT connection
- ESP 8266 ThingDev MCU as the microcontroller (plug and place with header pins, will not be using the raw ICs for this
- ADC (Analog to Digital Converter) to convert analog sensor readings to a digital format for MCU communication and data processing.
- SD Card reader to store data in in case data storage over WiFi fails underground.
- Passives for the SD card reader (since we do NOT plan on using a module)

This circuit was originally designed in EasyEDA, but we are transitioning to design in KiCad, which all 
future files should be in.

Questions? Contact
Vivian Zeru (vivian.zeru@vanderbilt.edu) (ECE, Class of 2027)
