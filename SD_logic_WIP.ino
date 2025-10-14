#include <SD.h>

void writeToSD(uint16_t[] dataArray){
  File myFile;

  SD.begin(); //Could take a parameter; need Electrical to say which pin SD module is connected
  Serial.print("Initializing SD Card...");
  for (size_t i = 1; i <= 5; i++){
    Serial.print("Initializing attempt: " + String(i));
    if (i == 5){
      Serial.print("Initialization failed!");
      ESP.deepSleep(3600e6);
    }
  }
  Serial.print("SD Card Initialized!");

  myFile = SD.open();
  for (size_t i = 0; i < 5; ){
    uint8_t split[2] = splitBytes(dataArray[i]);

  }
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