#include <SD.h>

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

uint16_t[] readFromSD(uint16_t[] dataArray){
  initializeSD();

  File myFile = SD.open("/");

  //Based on our naming scheme, here is how we will be tracking existing files.
  //Assume that Wifi will last long enough to transmit all data and delete all files.
  size_t fileNum = 0;
  while(SD.exists("data" + String(fileNum) + ".txt")){
    myFile = SD.open("data" + String(fileNum) + ".txt", FILE_READ);
    //TODO: add logic to read and combine bytes

    myFile.close();
    SD.remove("data" + String(fileNum) + ".txt"); //delete file once done
    ++fileNum;
  }

}

void initializeSD(){
  SD.begin(); //TODO: Could take a parameter; need Electrical to say which pin SD module is connected
  Serial.print("Initializing SD Card...");
  for (size_t i = 1; i <= 5; i++){
    Serial.print("Initializing attempt: " + String(i));
    if (i == 5){
      Serial.print("Initialization failed!");
      ESP.deepSleep(3600e6); //Sleep if we failed to initialize
    }
  }
  Serial.print("SD Card Initialized!");
}

uint8_t[] splitBytes(uint16_t twoBytes){
  uint8_t bytes[2];
  memcpy(bytes, &twoBytes, 2);
  return bytes;
}

uint16_t combineBytes(uint8_t[] bytes){
  uint16_t twoBytes;
  memcpy(twoBytes, &bytes, 2);
  return twoBytes;
}