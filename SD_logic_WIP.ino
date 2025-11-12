#include <SD.h>

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
readAllFromSD - Opens SD card to read a file into in series of 2 bytes for 10 bytes total
precondition - file to read only has 10 bytes, 5 sensors need 2 bytes each
postcondition - files read are deleted upon completion

@param dataArray - an array of size 5 (because we have 5 sensors) holding the data to write
TODO: REPLACE @param dataArray and use an ArrayList!
*/
// Opens SD card to read a file into in 2 bytes
// @param dataArray: the array holding the data to read
uint16_t[] readAllFromSD(uint16_t[] dataArray){
  initializeSD();

  File myFile = SD.open("/");

  //Based on our naming scheme, here is how we will be tracking existing files.
  //Assume that Wifi will last long enough to transmit all data and delete all files.
  size_t fileNum = 0;

  while(SD.exists("data" + String(fileNum) + ".txt")){
    readFromSD(dataArray, fileNum)
    SD.remove("data" + String(fileNum) + ".txt"); //delete file once done
    ++fileNum;
  }

  myFile.close();

  return dataArray;
}

//TODO: ADD METHOD HEADERS
uint16_t[] readFromSD(uint16_t[] dataArray, fileNum){
  //Read one file at a time
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

  return dataArray;
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
