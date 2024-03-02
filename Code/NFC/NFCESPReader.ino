// NFC Reader
// This code will read an NFC tag and store it in a variable using the ESP32-S3 Devkit C and RFID-RC522
// Function "readNFC" used to read the NFC tag
// Arduino IDE v2.3.2
// Author: Smart Quad EV Charger Development Team


#include <SPI.h>               // Include the SPI library for communication with peripherals
#include <MFRC522.h>           // Include the MFRC522 library for interacting with the RFID module

#define SS_PIN 10               // SDA
#define RST_PIN 14              // RST
#define SCK_PIN 367             // SCK
#define MOSI_PIN 39             // MOSI
#define MISO_PIN 13             // MISO

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create an instance of the MFRC522 class with specified SS and RST pins
String tagID = "";                 // Declare a String variable to store the tag ID

void setup() {
  Serial.begin(115200);         //Used for testing
  
  // Initialize SPI with custom pins
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);  // Begin SPI communication
  mfrc522.PCD_Init();             // Initialize the MFRC522 module

  //Serial.println("Ready to read NFC cards!");  //Used for Testing
}

void readNFC() {
  unsigned long startTime = millis();  // Record the start time
  bool cardRead = false;  // Flag to indicate if a card has been read

  while (millis() - startTime < 10000) {  // Continue checking for cards for up to 10 seconds
    // Look for new cards
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      //Serial.print("Card UID: "); //Used for Testing
      tagID = getTagID(mfrc522.uid.uidByte, mfrc522.uid.size);  // Get the tag ID and store it in the tagID variable
      //Serial.println(tagID);  //Used for Testing, Print ID Card
      cardRead = true;
      break;  // Exit the loop if card is read
    }
  }

  // When card id read wait 1 second
  if (cardRead) {
    delay(1000);
  }
  
  // If card is read return true if card is not read return false
  return cardRead;
}
