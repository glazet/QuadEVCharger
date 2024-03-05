#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <MFRC522.h>

#define TFT_DC 3
#define TFT_CS 36
#define TFT_MISO -1  // Not used in your setup
#define TFT_CLK 21
#define TFT_RST 47
#define TFT_MOSI 18

#define RFID_SS_PIN 10               // SDA
#define RFID_RST_PIN 19              // RST
#define RFID_SCK_PIN 8               // SCK
#define RFID_MOSI_PIN 17             // MOSI
#define RFID_MISO_PIN 20             // MISO

String tagID = "";

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

bool readNFC();
String getTagID(byte *buffer, byte bufferSize);

void setup() {
  Serial.begin(115200);
  SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
  mfrc522.PCD_Init();

  SPI.begin();
  tft.begin();
  tft.setRotation(3);  // Adjust the rotation if needed

  

  tft.fillScreen(ILI9341_BLACK); // Clear the screen

  Serial.println("Ready to read NFC cards and display on TFT!");
}

void loop() {
  // Display "Hello World" on TFT
  tft.setCursor(10, 50);  // Adjust the position
  tft.setTextSize(3);     // Adjust the text size
  tft.setTextColor(ILI9341_RED);
  tft.print("Hello World!");

  // Read NFC tag
  readNFC();
  Serial.print("TESTBBBB");
  Serial.println(tagID);

  
  // Display tag ID on TFT
  tft.setCursor(10, 100);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Card UID: ");
  tft.println(tagID);

  delay(5000); // Add a 5-second delay

  tft.fillScreen(ILI9341_BLACK); // Clear the screen for the next iteration
}

bool readNFC() {
  // Look for new cards
  Serial.print("TEST READ NFC");
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {    //Loop this function for 10 seconds
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { // Checking if NFCtag is present 
      tagID = getTagID(mfrc522.uid.uidByte, mfrc522.uid.size); //Format the tag id
      delay(1000);
      Serial.print("TESTES");
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
