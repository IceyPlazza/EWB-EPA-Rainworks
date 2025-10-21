#include <SD.h>

// Function to extract a specific byte from a 16-bit value
uint8_t getOneByte(uint16_t input, bool isLowerByte) {
  if (isLowerByte) {
    // LS-byte
    return input & 0xFF;
  } else {
    // MS-byte
    return (input >> 8) & 0xFF;
  }
}

writeFieldsToFile(uin16_t fields[5]) {
  for (int i = 0; i < 5; i++) {
    uint8_t upperByte = getOneByte(fields[i], false);
    uint8_t lowerByte = getOneByte(fields[i], true);
      
    dataFile.write(upperByte);
    dataFile.write(lowerByte);
  }
  dataFile.close();
  Serial.println("Inputs written successfully");
}


void setup() {
  Serial.begin(9600);

  SD.mkdir("/MyData");

  // some list of 5 input 16-bits
  uint16_t fields[5] = {1234, 5678, 9012, 3456, 7890};

  File dataFile = SD.open("/MyData/data.bin", FILE_WRITE);
  
  // looping through each field
  if (dataFile) {
    writeFieldsToFile(fields);
  } else {
    Serial.println("File not opened");
  }
}

void loop() {

}