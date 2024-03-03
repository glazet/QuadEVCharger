// NFC Reader
// This code will read an NFC tag and store it in a variable using the ESP32-S3 Devkit C and RFID-RC522
// Function "readNFC" used to read the NFC tag
// Arduino IDE v2.3.2
// Author: Smart Quad EV Charger Development Team


#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10     //SDA
#define RST_PIN 14    //RST
#define SCK_PIN 37    //SCK
#define MOSI_PIN 39   //MOSI
#define MISO_PIN 13   //MISO

MFRC522 mfrc522(SS_PIN, RST_PIN);
String tagID = "";    //Stores TagID

// Declare getTagID function before it's used
String getTagID(byte *buffer, byte bufferSize); // Define function getTag id used to format string
void readNFC();   //Define function readNFC used to read the nfc then calls function getTagID once tag is read

void setup() {
  Serial.begin(115200);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  Serial.println("Ready to read NFC cards!");
}

void loop() {
  Serial.print("Start");
  readNFC();
  Serial.print("Card UID: ");
  Serial.println(tagID);
  Serial.print("END");
  delay(2000);
}

bool readNFC() {
  // Look for new cards
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {    //Loop this function for 10 seconds
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { // Checking if NFCtag is present 
      tagID = getTagID(mfrc522.uid.uidByte, mfrc522.uid.size); //Format the tag id
      delay(1000);
      return true; //Return true if tag is read
      break;
      
    }
    return false; //Return false if no tag is read
  }
}

String getTagID(byte *buffer, byte bufferSize) {
  String tag = "";
  for (byte i = 0; i < bufferSize; i++) {
    tag += (buffer[i] < 0x10 ? "0" : "") + String(buffer[i], HEX);
  }
  return tag;
}
