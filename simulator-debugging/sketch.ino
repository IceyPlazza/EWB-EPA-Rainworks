#include "SD.h"


uint16_t dataArray[5] = {0, 1, 2, 3, 4};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  writeToSD(dataArray);
  uint16_t* result = readFromSD(0);
  for (int i = 0; i < 5; i++){
    Serial.println(dataArray[i]);
    Serial.println(*(result + i));
  }
}

void loop() {
  
}


/*
writeToSD - Opens SD card and creates a file to write data in series of 2 bytes for 10 bytes total
postcondition - a new file is created containing 10 bytes, as 5 sensors need 2 bytes each

@param dataArray - an array of size 5 (because we have 5 sensors) holding the data to write
*/
void writeToSD(uint16_t dataArray[]){
  initializeSD();//Initializes the SD card

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
    Serial.println("Splitting:");
    uint8_t split[2];
    splitBytes(dataArray[i], split);
    Serial.println(split[0]);
    Serial.println(split[1]);
    myFile.write(split, 2);
  }

  myFile.close(); //Must close file when we are done to save changes.
  Serial.println("Successfullly wrote file to SD");
}

/*
readFromSD - Opens SD card to read a file into in series of 2 bytes for 10 bytes total
precondition - file to read only has 10 bytes, 5 sensors need 2 bytes each

@return dataArray - an array of size 5 (due to 5 sensors) holding the data we read
*/
uint16_t* readFromSD(size_t fileNum){
  uint16_t dataArray[5];
  
  //Read one file at a time
  File myFile = SD.open("data" + String(fileNum) + ".txt", FILE_READ);
  //TODO: add logic to read and combine bytes
  for (int i = 0; i < 5; i++) {
    if (myFile.available()) {
      uint8_t bytes[2];
      bytes[0] = myFile.read();
      bytes[1] = myFile.read();

      dataArray[i] = combineBytes(bytes);
    }
  }

  return dataArray;
}

/*
initializeSD - a void helper method to initialize the SD card. 
*/
bool initializeSD(){
  Serial.print("Initializing SD Card...\n");
  for (size_t i = 1; i <= 5; i++){
    int success = SD.begin(4); //TODO: Could take a parameter; need Electrical to say which pin SD module is connected
    Serial.print("Initializing attempt: " + String(i));
    if (success == 1){
      Serial.print("\nSD Card Initialized!\n");
      return true;
    }
    delay(5000); //Was told it needs 5-10 sec to connect
    
  }

  Serial.print("Initialization failed!");
	return false;
    
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