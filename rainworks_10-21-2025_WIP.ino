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
int deepSleepTime = 30e6; //Variable to determine how long to sleep for: 30e6 means 30 seconds
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
  writeMain();

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
writeToSD - Opens SD card and creates a file to write data in series of 2 bytes for 10 bytes total
postcondition - a new file is created containing 10 bytes, as 5 sensors need 2 bytes each

@param dataArray - an array of size 5 (because we have 5 sensors) holding the data to write
*/
void writeToSD(uint16_t[] dataArray){
  initializeSD()//Initializes the SD card

  File myFile = SD.open("/"); //Opens root directory; assuming SD card only has root dir

  //Creating a file to write to
  //Logic to correctly identify different files and available filenames
  size_t fileNum = 0;
  while(SD.exists("data" + String(fileNum) + ".txt")){
    ++fileNum;
  }
  myFile = SD.open("data" + String(fileNum) + ".txt", FILE_WRITE);

  //Write data to file in terms of bytes
  for (size_t i = 0; i < 5; i++){
    uint8_t split[2] = splitBytes(dataArray[i]);
    myFile.write(split, 2);
  }

  myFile.close(); //Must close file when we are done to save changes.
}

/*
readFromSD - Opens SD card to read a file into in series of 2 bytes for 10 bytes total
precondition - file to read only has 10 bytes, 5 sensors need 2 bytes each
postcondition - files read are deleted upon completion

@param dataArray - an array of size 5 (because we have 5 sensors) holding the data to write
*/
uint16_t[] readFromSD(uint16_t[] dataArray){
  initializeSD();

  File myFile = SD.open("/");

  //Based on our naming scheme, here is how we will be tracking existing files.
  //Assume that Wifi will last long enough to transmit all data and delete all files.
  size_t fileNum = 0;
  while(SD.exists("data" + String(fileNum) + ".txt")){
    myFile = SD.open("data" + String(fileNum) + ".txt", FILE_READ);
    //TODO: add logic to read and combine bytes
    for (int i = 0, i < 5, i++) {
      if (myFile.available()) {
        uint8_t bytes[2];
        uint8_t bytes[0] = myFile.read();
        uint8_t bytes[1] = myFile.read();

        dataFields[i] = combineBytes(bytes);
      }
    }
    myFile.close();
    SD.remove("data" + String(fileNum) + ".txt"); //delete file once done
    ++fileNum;
  }
  return dataArray;
}

/*
  writeMain -- function to determine whether to write directly to ThingSpeak or to SD card
  //TODO: ADD TO THIS; DONT HESITATE TO EDIT OR CHANGE LOGIC IF YOU DISCOVER SOMETHING BETTER
*/
int numFilesInSD = 0; // Temporary int for checking how many files in SD to read until the check-if-file-exists
                              // implemented in the read sd card method

void writeMain(){
  //First, check if we have wifi.
  if (checkWifiStatus()) {
    //If we have wifi, check if we have files in SD to read and send
    for (int i = numFilesInSD; i > 0; ++i) { // Temporary int placeholder to see if files exist
      writeToThingSpeak(readFromSD(5)); // Looks like the readFromSD reads multiple files? Not sure how that would work
      delay(120); // 2 min delay
    }

    // Read and send form sensors
   
  }
    //Send our new readings
  //If we do not have wifi
    //Read from sensors
    //Write to SD card
}

/*
initializeSD - a void helper method to initialize the SD card. 
Will sleep and prematurely stop the program if SD card cannot be initialized.
*/
void initializeSD(){
  //TODO: Could take a parameter; need Electrical to say which pin SD module is connected
  Serial.print("Initializing SD Card...");
  for (size_t i = 1; i <= 5; i++){
    int success = SD.begin(); 
    Serial.print("Initializing attempt: " + String(i));
    if (success == 1){
      Serial.print("SD Card Initialized!");
      return;
    }
    delay(5000); //Was told it needs 5-10 sec to connect
    if (i == 5){
      Serial.print("Initialization failed!");
      ESP.deepSleep(3600e6); //Sleep if we failed to initialize
    }
  }
}

/*
splitBytes - a helper method that takes a uint16_t value and split it into two seperate bytes
@param twoBytes - a uint16_t value
@return - a uint8_t array with two elements. Each element is a separate byte of data.
*/
uint8_t[] splitBytes(uint16_t twoBytes){
  uint8_t bytes[2];
  memcpy(bytes, &twoBytes, 2);
  return bytes;
}

/*
combineBytes - a helper method that takes an array of 2 bytes and merge them together in a new data type.
@param bytes - a uint8_t array of size 2
@return - a uint16_t value that's the result of merging the bytes in our parameter
*/
uint16_t combineBytes(uint8_t[] bytes){
  uint16_t twoBytes;
  memcpy(twoBytes, &bytes, 2);
  return twoBytes;
}



