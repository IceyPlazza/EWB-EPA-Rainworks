//Libraries to use
#include "ThingSpeak.h"
#include "SPI.h"
#include <ESP8266WiFi.h>
#include <SD.h>

//ThinkSpeak API
unsigned long myChannelNumber = 2637657;
const char * myWriteAPIKey = "08KCACI3TEVK7VSE";

//Wifi connection we expect to be using
char ssid[] = "vuDevices";   // your network SSID (name)
char pass[] = "Acorn1873";   // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
const int ADC_CS_PIN = 15;   //Pin connected to CS/SHDN pin on MCP3008
WiFiClient  client;

//Create Object of SPI class (This is how we are reading from sensors)
SPIClass mySPI;
// Sets the speed of SPI data transmission (Hz), order of bits transmitted, and the mode which determines clock polarity and phase
SPISettings adcSettings(100000, MSBFIRST, SPI_MODE0);

//Extra needed variables
bool isRaining = false; //Check if raining or not
int rainingFast = 20; //Value if it's raining fast. Can figure out later
int deepSleepTime = 30e6; //Variable to determine how long to sleep for
int prevRainValue = 0; //Variable to store and check a prior raining value
uint16_t sensorValues[5]; //Array to store all sensor readings

void setup() {

  //Initial setup
  Serial.begin(115200);
  pinMode(ADC_CS_PIN, OUTPUT);
  mySPI.begin(); 
  mySPI.beginTransaction(adcSettings);

  //Series of actions to take before we deepSleep.
  Serial.println("\nStarting up...");
  WiFi.mode(WIFI_STA);
  connectWifi();
  ThingSpeak.begin(client);
  writeAll();

  //Let's wait an extra tenth of a second just in case there's any delays we might have to deal with
  Serial.println("\nEntering Deep Sleep for " + String(deepSleepTime) + " seconds.");
  ESP.deepSleep(deepSleepTime);
}

void loop() {
  //We supposedly won't need loop if using deepSleep
}

/*
////////////////////////////////////////////////
  Please keep all helper methods below this line
///////////////////////////////////////////////
*/

/*
  readFromADC -- function to help read data from each respective sensor connected to the respective channels
  @param channel - the channel to which the sensor should be attached
  @return - the reading from the sensor
*/
uint16_t readFromADC(uint8_t channel) {
  mySPI.beginTransaction(adcSettings);
  
    // MCP3008 command format: Start bit (1), Single-ended mode (1), Channel (3 bits)
    uint16_t command = 0xc000 | (channel << 11);

    // Select the MCP3008 by pulling CS low
    digitalWrite(ADC_CS_PIN, LOW);

    // Send the command
    uint16_t result = mySPI.transfer16(command);

    // Deselect the MCP3008 by pulling CS high
    digitalWrite(ADC_CS_PIN, HIGH);
    
    mySPI.endTransaction();

    // Extract the 10-bit ADC value from the result
    uint16_t adcValue = (result<<1) & 0x03FF;

    return adcValue;
}

/*
  connectWifi -- function to establish a Wifi connection
*/
void connectWifi(){

  //Having this here just for in case a different non-registered board was used.
  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  // Connect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Establishing WiFi connection: ");

    //Attempt to connect to Wifi 4 times. If all 4 fails, we will enter extended deep sleep, since it is likely that Wifi is currently down.
    for (size_t i = 0; i < 4; i++){
      if (WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
        Serial.print(".");
        delay(10000); //Was told it needs 5-10 sec to connect
      } else {
        Serial.println("\nConnected.");
        break;
      }
    } 

    //NOTE: If no wifi, we should write to SD card
    if (WiFi.status() != WL_CONNECTED){
      Serial.println("Wifi unavailable.");
    }
  }
}

/*
  readSensors -- helper method to read and store all sensor values into an array
  @param sensorValues -- A uint16_t array with 5 array slots (as we have 5 sensors)
*/
void readSensors(uint16_t sensorValues[]){
  // Runs 5 times to set each of the 5 field values
  // ADC channels are zero based indexing
  for (size_t i = 0; i < 5; ++i) {

    // Measure Analog Input (A0)
    sensorValues[i] = readFromADC(i);
    delay(100);

    //Checks if it's pin 4 and sees if it's raining fast or not.
    if(i==4){
      if( (prevRainValue-sensorValues[i]) < rainingFast){
        isRaining = false;
        deepSleepTime = 3000e6;
      }
      else((prevRainValue-sensorValues[i]) > rainingFast){
        isRaining = true;
        deepSleepTime = 15e6;
      }
      prevRainValue = sensorValues[i]
    }
  }
}

/*
  writeToThingSpeak -- function to send all data in an array to ThingSpeak
  @param dataArray -- A uint16_t array with 5 array slots (as we have 5 sensors)
*/
void writeToThingSpeak(uint16_t dataArray[]){
  
  for (size_t i = 0; i < 5; ++i){
    ThingSpeak.setField(i+1, dataArray[i]); //ThingSpeak is 1-based indexing
  }

  //Write all 5 ADC values to Thingspeak at the same time
  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  //HTTP Code 200 means success, anything else means there was an error of some sort.
  if (httpCode == 200) {
    Serial.print("\nWriting to Channel 1 Field:\n");
    for (size_t i = 0; i < 5; i++){
      Serial.print(i+1);
      Serial.print(" with value ");
      Serial.println(dataArray[i]);
    }
  }
  else {
    Serial.println("\nProblem writing to Channel 1 Field ");
    Serial.println("HTTP error code " + String(httpCode));
  }
}

/*
  writeMain -- function to determine whether to write directly to ThingSpeak or to SD card
  //TODO: OVERHAUL THIS
*/
void writeMain(){
  
}