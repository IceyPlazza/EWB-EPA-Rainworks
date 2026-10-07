//Libraries to use
#include "ThingSpeak.h"
#include "SPI.h"
#include "SD.h"
#include <ESP8266WiFi.h>

void setup() {
  // put your setup code here, to run once:
  uint16_t testArr[] = {12,15,7,21,18}; 
  // {00000000 00001100, 00000000 00001111, 00000000 00000111, 00000000 00010101, 00000000 00010010}

  initializeSD();

  writeToSD(testArr);

}

void loop() {
  // put your main code here, to run repeatedly:

}

/**
  splitBytes - a helper method that takes a uint16_t value and split it into two seperate bytes
  @param twoBytes - a uint16_t value
  @param *bytes - a pointer looking at an array of two uint8_t bytes
*/
void splitBytes(uint16_t twoBytes, uint8_t *bytes){
    memcpy(bytes, &twoBytes, 2);
}

/**
  combineBytes - a helper method that takes an array of 2 bytes and merge them together in a new data type.
  @param bytes - a uint8_t array of size 2
  @return - a uint16_t value that's the result of merging the bytes in our parameter
*/
uint16_t combineBytes(uint8_t bytes[]){
  uint16_t twoBytes;
  memcpy(&twoBytes, bytes, 2);
  return twoBytes;
}

/**
  initializeSD - a void helper method to initialize the SD card. Goes to deepSleep if failed to initialize.
*/
void initializeSD(){

  int success = SD.begin(4);

  if (success == 1){
    Serial.println("SD already initialized!");
    return;
  }

  Serial.println("Initializing SD Card...");

  for (size_t i = 1; i <= 5; i++){
    success = SD.begin(4); //TODO: Could take a parameter; need Electrical to say which pin SD module is connected
    Serial.println("Initializing attempt: " + String(i));
    if (success == 1){
      Serial.println("SD Card Initialized!");
      return;
    }

    delay(5000); //Was told it needs 5-10 sec to connect
  }

  Serial.println("Initialization failed!");
	Serial.println("\nEntering Deep Sleep for " + String(deepSleepTime / 1000000) + " seconds.");
  ESP.deepSleep(deepSleepTime);
	return;
    
}

/**
  writeToSD - Opens SD card and creates a file to write data in series of 2 bytes for 10 bytes total
  postcondition - a new file is created containing 10 bytes, as 5 sensors need 2 bytes each

  @param dataArray - an array of size 5 (because we have 5 sensors) holding the data to write
*/
void writeToSD(uint16_t dataArray[]){

   initializeSD();

  //Creating a file to write to
  //Logic to correctly identify different files and available filenames
  size_t fileNum = 0;
  while(SD.exists("data" + String(fileNum) + ".txt")){
    ++fileNum;
  }
  File myFile = SD.open("data" + String(fileNum) + ".txt", FILE_WRITE);

  //Write data to file in terms of bytes
  for (size_t i = 0; i < 5; i++){
	uint8_t split[2];
    splitBytes(dataArray[i], split);
    myFile.write(split, 2);
  }

  myFile.close(); //Must close file when we are done to save changes.
}

