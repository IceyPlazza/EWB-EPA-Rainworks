Steps to Upload Code to Board for the First Time 

1. Use Arduino IDE, not web browser 

2. Enter Arduino IDE 

3. Go to File -> Preferences -> Additional board manager URLs and paste the following 

4. http://arduino.esp8266.com/stable/package_esp8266com_index.json 

5. Go to Sketch -> Include Library -> Manage Libraries and add the Thingspeak library 

Note: The SPI and Wifi libraries should have been added already from Step 3 

- Register device with Vanderbilt in order to connect to Wifi 

- This requires running a script to find the MAC address of the Thing Dev (Should print on Serial Monitor) 

- Use this link: https://wifi.vanderbilt.edu/vunetid.html  

- Connect the Thing Dev to the computer using the USB connector 

- Ensure that the channel number and write API key are correct 

 

Steps to Read Board’s Serial Messages 

1. Go to Tools -> Serial Monitor 

2. In the monitor, navigate to where it has a value and “baud” 

3. Set the baud value to the same as the declared value in Serial.begin(value); 

4. Last we checked, it was set to 115200 

5. Ensure that the COM port is set correctly and labeled as “SparkFun ESP8266 Thing Dev” 

6. Start the board.  

If nothing is showing up in the Serial Monitor, turn the board on and off again. 

Alternatively, switch COM ports then switch back to the correct COM port. 
