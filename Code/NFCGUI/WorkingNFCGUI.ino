#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


/////////////////////////////////////////////////////////////////////////////////////
// LCD Pin Definition
/////////////////////////////////////////////////////////////////////////////////////

#define TFT_DC 35                                             // DC
#define TFT_CS 8                                            // CS
#define TFT_MISO -1                                          // Not Used
#define TFT_CLK 12                                           // CLK
#define TFT_RST 11                                           // RST
#define TFT_MOSI 37                                          // MOSI

/////////////////////////////////////////////////////////////////////////////////////
// RFPID Pin Definition (NFC Reader)
/////////////////////////////////////////////////////////////////////////////////////

#define RFID_SS_PIN 10                                        // SDA
#define RFID_RST_PIN 14                                       // RST
#define RFID_SCK_PIN 36                                        // SCK
#define RFID_MOSI_PIN 38                                     // MOSI
#define RFID_MISO_PIN 13                                      // MISO


/////////////////////////////////////////////////////////////////////////////////////
// WiFi Initialization
/////////////////////////////////////////////////////////////////////////////////////

const char *ssid = "notavirus";                               // WiFi Hostname
const char *password = "Hotspot22";                           // WiFi Password

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

/////////////////////////////////////////////////////////////////////////////////////
// Variable declaration
/////////////////////////////////////////////////////////////////////////////////////

        
bool startUpBool = false;                                      // Is start up complete     
 
int cardRead = false;                                          // Is NFC card read or does user want to exit    

int buttonNum = 0;                                             // Used in function checkButtonPress returns button number
int button1 = -1;                                              // Button 1
int button2 = -1;                                              // Button 2
int button3 = -1;                                              // Button 3
int button4 = 19;                                              // Button 4

int LEDG = 0;                                                  // RED LED
int LEDR = 45;                                                 // Green LED

int pilotFB;

int prioPlug[4] = {};
String nfcInfo[4] = {"", "", "", ""};

//Plug Arrays {Car Status, Relay Status, priority}
//Default state -1
// Car Status: 
// Relay Status: 0, 1 (0 = OFF, 1 = ON)
// Priority: 0, 1, 2, 3 (0 = Charging)

int plugA[2] = {-1, -1, -1}; 
int plugB[2] = {-1, -1, -1}; 
int plugC[2] = {-1, -1, -1}; 
int plugD[2] = {-1, -1, -1}; 


const int plug1 = 15;
const int plug2 = 16;
const int plug3 = 17;
const int plug4 = 18;

const int RLA = 42;
const int RLB = 41;
const int RLC = 40;
const int RLD = 39; 

bool RLAS = false;
bool RLBS = false;
bool RLCS = false;
bool RLDS = false;


String tagID = "";                                            //Used for reading the tag id
String colorb;                                                //Setting Color used for Buttons

/////////////////////////////////////////////////////////////////////////////////////
// LCD and RFID Initialization
/////////////////////////////////////////////////////////////////////////////////////

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);


/////////////////////////////////////////////////////////////////////////////////////
// Variable declaration
/////////////////////////////////////////////////////////////////////////////////////

int readNFC();                                               // Function used to read NFC waits 10 seconds
String getTagID(byte *buffer, byte bufferSize);               // Converting ID into a String
void welcomescreen();                                         // Main Screen
void drawButton(String label, int x, int y);                  // Used to create boxes on LCD representing buttons
bool TapCardScreen(int plugName);                              // Prompt users to tap their NFC Card
void FillScreenBlank();                                       // Clear Screen
int checkButtonPress();                                       // Checks which button is pressed
void onLED(String LEDcolor);                                  // Turns on LED input "G" (Green LED) or "R" (Red LED)
void offLED();                                                // Turns all LEDs off
bool startUP();                                               // On power up begin this function
bool iswifiUP();                                              // Check if WiFi is Up
void getTime();                                               // Prints the time on the top left of the LCD
int pilotRead(int plugNum);                                   // Checks each plug and assigns priority level
void prioPlugShift();

/////////////////////////////////////////////////////////////////////////////////////
// Void Setup
/////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);


  // Setting corrisponding pins to output or input
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);

  pinMode(RLA, OUTPUT);
  pinMode(RLB, OUTPUT);
  pinMode(RLC, OUTPUT);
  pinMode(RLD, OUTPUT);

  

  // Begin SPI for NFC reader and LCD
  SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
  mfrc522.PCD_Init();
  SPI.begin();
  tft.begin();

  // Set LCD rotation and Clear screen
  tft.setRotation(1);
  FillScreenBlank();

  // Begin Initial start up

  
  while (!startUP()) {                            // If starup fails keep calling function
    startUP();
  }

  welcomescreen();                                          // Draw welcome screen on LCD

}

/////////////////////////////////////////////////////////////////////////////////////
// Void Loop
/////////////////////////////////////////////////////////////////////////////////////

void loop() {
  unsigned long ChTime = millis() + 15000;
  while (millis() < ChTime) {
    delay(1000);
      
    pilotFB = pilotRead(plug1);
    if (pilotFB == 12  || pilotFB == 0) {
      break;
    }
    pilotFB = pilotRead(plug2);
    if (pilotFB == 12  || pilotFB == 0) {
      break;
    }
    pilotFB = pilotRead(plug3);
    if (pilotFB == 12  || pilotFB == 0) {
      break;
    }
    pilotFB = pilotRead(plug4);
    if (pilotFB == 12  || pilotFB == 0) {
      break;
    }

    //checkButtonPress();
    Serial.print(prioPlug[0]);
    Serial.print(prioPlug[1]);
    Serial.print(prioPlug[2]);
    Serial.println(prioPlug[3]);

    //if (buttonNum !=  0) {

    
    
    //  delay(500);
    //  welcomescreen();
    //}

  
  }
  getTime();
  iswifiUP();
  Serial.println("////////////////////////////");
  Serial.println("////////////////////////////");
  for (int i = 0; i < 4; ++i) {
    if (prioPlug[3] == 0) {
      RLAS = false;
      RLBS = false;
      RLCS = false;
      RLDS = false;
      break;
    }

    else if(prioPlug[i] > 0) {
      controlRelays(prioPlug[i]);
      prioPlug[i] = 0;
      Serial.print("Loop");
      for (int i = 0; i < 4 - 1; ++i) {
        prioPlug[i] = prioPlug[i + 1];
      }
      prioPlug[4 - 1] = 0;  // Set the last element to 0 (or any default value)
      Serial.print(prioPlug[0]);
      Serial.print(prioPlug[1]);
      Serial.print(prioPlug[2]);
      Serial.print(prioPlug[3]);
      //Serial.println(prioPlug[i]);
      //Serial.println(": Prioplug");
      break;

    }
  }
  


}

/////////////////////////////////////////////////////////////////////////////////////
// Read NFC & getTaGID
/////////////////////////////////////////////////////////////////////////////////////

int readNFC() {

  // Input: None
  // Output: 1 (Card is read)
  //         2 (Exit button is pressed)
  //         3 (Card is not read reached end wait time)
  // Adjustable variables: Timeout (Desired wait time)
  // Description: Once function is called NFC tag reader wait for set amount of time (timeout) until
  //              card approaches reader. If tag is read call getTagID function. This function will
  //              output the tagID to a string and return it. Then return true and break the while
  //              loop. If tag is not read return false.


  unsigned long startTime = millis();                                         // Delay variable
  int timeout = 5000;                                                         // Desired Time wait
  while (millis() - startTime < timeout) {                                    //Loop this function for desired time wait
    
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {   // Check if NFCtag is present 
      tagID = getTagID(mfrc522.uid.uidByte, mfrc522.uid.size);                // Format tag id
      
      return 1;                                                               //Return true if tag is read
      break;
      
    }
    if (checkButtonPress() == 4) {                                           // Check if exit button is pressed
      return 2;                                                              // If exit button is pressed return 2
    }
  }
  return 3;                                                              // Return false if tag is not read
}

String getTagID(byte *buffer, byte bufferSize) {                             // Format tag ID
  String tag = "";
  for (byte i = 0; i < bufferSize; i++) {
    tag += (buffer[i] < 0x10 ? "0" : "") + String(buffer[i], HEX);
  }
  return tag;
}

/////////////////////////////////////////////////////////////////////////////////////
// FillScreenBlank
/////////////////////////////////////////////////////////////////////////////////////

void FillScreenBlank() {

  // Input: None
  // Output: None
  // Adjustable Variables: None
  // Description: Function draws a rectangle to clear the LCD    

  tft.fillRect(0, 0, tft.width(), tft.height(), ILI9341_BLACK);             // Draw rectangle to clear screen
  

}

/////////////////////////////////////////////////////////////////////////////////////
// welcomescreen

/////////////////////////////////////////////////////////////////////////////////////

void welcomescreen() {

  // Input: None
  // Output: None
  // Adjustable Variables: None
  // Description: Ensure screen is cleared. Draw the welcome screen with 4 buttons at the bottom.
  //              The 4 buttons at the bottom refer to each plug.

  FillScreenBlank();
  
  // Display welcome text
  tft.setCursor(65, 75);
  tft.setTextSize(2.5);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Welcome EV Owner");

  // Draw buttons
  drawButton("Plug 1", 20, 185);
  drawButton("Plug 2", 97, 185);
  drawButton("Plug 3", 173, 185);
  drawButton("Plug 4", 250, 185);

  if (RLAS == true) {
    drawButtonC("Plug 1", 20, 185);
  }
  else if (RLBS == true) {
    drawButtonC("Plug 2", 97, 185);
  }
  else if (RLCS == true) {
    drawButtonC("Plug 3", 173, 185);
  }
  else if (RLDS == true) {
    drawButtonC("Plug 4", 250, 185);
  }

  // Get Time

  getTime();
  
}

/////////////////////////////////////////////////////////////////////////////////////
// drawButton
/////////////////////////////////////////////////////////////////////////////////////

void drawButton(String label, int x, int y) {

  // Input: label (Label of buttons)
  //        int x (position of button x axis)
  //        int y (position of button y axis)
  // Output: None
  // Adjustable Variables: buttonWidth (Desired button width in pixels)
  //                       buttonHeight (Desired button height in pixels)
  // Description: Draw a box representing a button at position x, y. Each button will
  //              have a solid navy background and text will be in white.

  int buttonWidth = 62;
  int buttonHeight = 40;
  
  // Draw button background
  tft.drawRect(x + 1, y + 1, buttonWidth - 2, buttonHeight - 2, ILI9341_WHITE);
  tft.fillRect(x + 2, y + 2, buttonWidth, buttonHeight, ILI9341_NAVY);

  // Draw button label
  tft.setCursor(x + 10, y + 18);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.print(label);
}

/////////////////////////////////////////////////////////////////////////////////////
// TapCardScreen
/////////////////////////////////////////////////////////////////////////////////////

bool TapCardScreen(int plugName) {

  // Input: None
  // Output: None
  // Adjustable Variables: None
  // Description: Ensure screen is cleared. Promt user to tap their card. Call
  //              function readNFC. If tag is read print "Card Read Successful"
  //              and light up green LED. If card is not read print "Session
  //              timed out" and light up red LED.


  // Clear screen
  FillScreenBlank();

  // Promt user to tap their card
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(75,75);
  tft.setTextSize(2);
  tft.print("Please Tap Card");
  tft.setCursor(0, 170);
  
  

  if (plugName == plug1) {
    tft.print("Plug: 1");
  }

  if (plugName == plug2) {
    tft.print("Plug: 2");
  }
  if (plugName == plug3) {
    tft.print("Plug: 3");
  }
  if (plugName == plug4) {
    tft.print("Plug: 4");
  }

  // Print the Time
  getTime();

  // Draw an exit button to go back to the welcome page
  drawButton("EXIT", 250, 160);
  
  // Call function readNFC
  cardRead = readNFC();

  // If card is read
  if (cardRead == 1) {
    tft.setTextSize(2);
    tft.setCursor(40, 100);
    tft.print("Card Read Successfull");
    tft.setCursor(40, 135);
    tft.print("TAG ID: ");
    tft.setCursor(140, 135);
    tft.print(tagID);
    onLED("G");
    delay(2000);
    offLED();
    Serial.print("PlugName -- ");
    Serial.print(plugName);
    if (plugName == plug1 && nfcInfo[0].isEmpty()) {
      nfcInfo[0] = tagID;
      Serial.println(nfcInfo[0]);
    }

    if (plugName == plug2  && nfcInfo[1].isEmpty()) {
      nfcInfo[1] = tagID;
    }
    if (plugName == plug3  && nfcInfo[2].isEmpty()) {
      nfcInfo[2] = tagID;
    }
    if (plugName == plug4  && nfcInfo[3].isEmpty()) {
      nfcInfo[3] = tagID;
    }

    tagID = "";
    FillScreenBlank();
    return true;
  }

  // If user has pushed exit button
  if (cardRead == 2) {
    return false;
  }

  // If NFC card has not been read
  else {
    FillScreenBlank();
    //tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(65, 75);
    tft.setTextColor(ILI9341_RED);
    tft.print("Session Timed Out");
    onLED("R");
    delay(2000);
    offLED();
    FillScreenBlank();
    return false;
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////
// checkButtonPress
/////////////////////////////////////////////////////////////////////////////////////

int checkButtonPress() {

  // Input: None
  // Output: buttonNum (Return button number that was pressed)
  // Adjustable Variables: None
  // Description: Read each button pin and check if button is pressed return the
  //              button number. If no button is pressed return 0

/*
  int Sbutton1 = digitalRead(button1);
  delay(50);
  int Sbutton2 = digitalRead(button2);
  delay(50);
  int Sbutton3 = digitalRead(button3);
  delay(50);
  */
  int Sbutton4 = digitalRead(button4);
  delay(50);


/*
  if (Sbutton1 == LOW) {
      Serial.println("Button 1 Pressed");
      buttonNum = 1;
  }

  else if (Sbutton2 == LOW) {
      Serial.println("Button 2 Pressed");
      buttonNum = 2;
  }

  else if (Sbutton3 == LOW) {
      Serial.println("Button 3 Pressed");
      buttonNum = 3;
  }
  */

  if (Sbutton4 == LOW) {
      Serial.println("Button 4 Pressed");
      buttonNum = 4;    
  }
  
  
  else {
    buttonNum = 0;
  }
  return buttonNum;
}

/////////////////////////////////////////////////////////////////////////////////////
// onLED
/////////////////////////////////////////////////////////////////////////////////////

void onLED(String LEDcolor) {

  // Input: LEDcolor (Either "G" for Green LED or "R" for Red LED)
  // Output: None
  // Adjustable Variables: None
  // Description: Light up green or red led
  
  if (LEDcolor == "G") {
    digitalWrite(LEDG, HIGH);
  }

  if (LEDcolor == "R") {
    digitalWrite(LEDR, HIGH);

  }
}

/////////////////////////////////////////////////////////////////////////////////////
// offLED
/////////////////////////////////////////////////////////////////////////////////////

void offLED() {

  // Input: None
  // Output: None
  // Adjustable Variables: None
  // Description: Turn off all LEDs

  digitalWrite(LEDG, LOW);
  digitalWrite(LEDR, LOW);
}

/////////////////////////////////////////////////////////////////////////////////////
// startUp
/////////////////////////////////////////////////////////////////////////////////////


bool startUP() {

  // Input: None
  // Output: None
  // Adjustable Variables: None
  // Description: On power up this function is called. It will clear the screen
  //              then connect to wifi. It will wait 10 seconds for wifi to
  //              connect. If timeout is reached startUp fails. 

  // Clear screen and promt "Begin initial start up"
  FillScreenBlank();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(20, 75);
  tft.print("Begining Initial Startup");
  delay(500);

  // Clear screen and promt "Connecting to wifi"
  

  // Connect to wifi
  WiFi.begin(ssid, password);
  unsigned long timeout = millis() + 10000;
  
  // Check if wifi is connected
  bool CWifi = false;
  while (millis() < timeout) {
    if (WiFi.status() == WL_CONNECTED) {
      FillScreenBlank();
      tft.setTextSize(2);
      tft.setCursor(70, 75);
      tft.print("WiFi Connected");
      onLED("G");
      delay(2000);
      offLED();
      CWifi = true;
      // Break loop once wifi is connected
      break;
    }
  }
  //

  // WiFi connection failed within the timeout
  if (WiFi.status() != WL_CONNECTED) {
    FillScreenBlank();
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_RED);
    tft.setCursor(30, 75);
    tft.print("WiFi Failed to Connect");
    onLED("R");
    delay(2000);

    // Reset timeout and wait before retrying
    timeout = millis() + 10000;
    while (millis() < timeout) {
    
  }

    FillScreenBlank();
    offLED();
    return false;
  }

  FillScreenBlank();
  timeClient.begin();
  timeClient.setTimeOffset(-8*3600);
  getTime();
  return true;

  
}

/////////////////////////////////////////////////////////////////////////////////////
// iswifiUP
/////////////////////////////////////////////////////////////////////////////////////

bool iswifiUP () {

  // Input: None
  // Output: None
  // Adjustable Variables: None
  // Description: Checks if wifi is connected, if it is not connected call function
  //              startup.

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  else {
    startUP();
  }

}

/////////////////////////////////////////////////////////////////////////////////////
// getTime
/////////////////////////////////////////////////////////////////////////////////////

void getTime() {
  timeClient.update();
  String currentTime = timeClient.getFormattedTime();
  int index = currentTime.indexOf(":");
  String hoursMinutes = currentTime.substring(0, index + 3);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  
  tft.fillRect(0, 0, 150, 40, ILI9341_BLACK);
  tft.print(hoursMinutes);

}

/////////////////////////////////////////////////////////////////////////////////////
// pilotRead
/////////////////////////////////////////////////////////////////////////////////////

int pilotRead(int plugNum) {
  int controlInput = analogRead(plugNum);
  float voltage = (controlInput / (float)4095) * 3.3;
  

  // Display the corresponding voltage value
  bool isPlugNumInArray = false;
  int plugIndex = -1;  // To store the index of plugNum in the array
  int plugIndexB = 0;
  bool nfcRead = false;

  // Check if plugNum is in the array
  for (int i = 0; i < 4; ++i) {
    if (prioPlug[i] == plugNum) {
      isPlugNumInArray = true;
      plugIndex = i;
      break;
    }
  }
  //Django addition: maybe the block above this and below this can be combined since they are both just searching for which plug is activate
  // Check the index of plugNum to update nfcInfo
  if (plugNum == plug1) {
    plugIndexB = 0;
  } else if (plugNum == plug2) {
    plugIndexB = 1;
  } else if (plugNum == plug3) {
    plugIndexB = 2;
  } else if (plugNum == plug4) {
    plugIndexB = 3;
  }

  // Update nfcInfo based on the nfcRead condition
  if (!nfcRead && !nfcInfo[plugIndexB].isEmpty()) {
    nfcRead = true;
    //Serial.print("NFC read set to true: ");
    //erial.println(plugIndexB);
    //test
  }

  // Update the array based on voltage levels
  if (voltage > 3.18 && voltage < 3.38) {
    // If plugNum is in the array, remove it and shift the values to the left
    if (isPlugNumInArray) {
      for (int i = plugIndex; i < 4 - 1; ++i) {
        prioPlug[i] = prioPlug[i + 1];
      }
      prioPlug[4 - 1] = 0;  // Set the last element to 0 (or any default value)
    }
    nfcInfo[plugIndexB].clear();
    
    return 12;
  } else if (!nfcRead && !isPlugNumInArray && voltage > 2.7 && voltage < 2.9) {
    Serial.println("9V: ");
    Serial.print(nfcInfo[plugIndexB]);

    if (TapCardScreen(plugNum)) {
      // Check if plugNum is not already in the array
      bool isNewPlug = true;
      for (int i = 0; i < 4; ++i) {
        if (prioPlug[i] == plugNum) {
          isNewPlug = false;
          break;
        }
      }

      if (isNewPlug) {
        for (int i = 0; i < 4 - 1; ++i) {
          prioPlug[i] = prioPlug[i + 1];
        }

        // Assign priority level to the new plug
        prioPlug[4 - 1] = plugNum; // Adjust priority level as needed
      }
    } else {
      welcomescreen();
      return -1;
    }
    welcomescreen();
    return 9;
  } else if (voltage > 2.37 && voltage < 2.57) {
    return 6;
  } else if (voltage > 2.05 && voltage < 2.25) {
    return 3;
  } else if (voltage > 1.5 && voltage < 1.7) {
    // If plugNum is in the array, remove it and shift the values to the left
    if (isPlugNumInArray) {
      for (int i = plugIndex; i < 4 - 1; ++i) {
        prioPlug[i] = prioPlug[i + 1];
      }
      prioPlug[4 - 1] = 0;  // Set the last element to 0 (or any default value)
    }
    nfcInfo[plugIndexB].clear();
    //Serial.print(plugIndexB);
    //Serial.println("nfc Clear");
    
    return 0;
  } else if (voltage > 0.03 && voltage < 0.73) {
    // If plugNum is in the array, remove it and shift the values to the left
    if (isPlugNumInArray) {
      for (int i = plugIndex; i < 4 - 1; ++i) {
        prioPlug[i] = prioPlug[i + 1];
      }
      prioPlug[4 - 1] = 0;  // Set the last element to 0 (or any default value)
    }
    nfcInfo[plugIndexB].clear();
    //Serial.print(plugIndexB);
    //Serial.println("nfc Clear");
    return 1;
  } else if (nfcRead && !isPlugNumInArray) {
      prioPlug[3] = plugNum;
  } else {
    return -1; // Return an indication of an unknown value
  }

  delay(1000); // Adjust the delay based on your requirements
}


void prioPlugShiftResetLast(int plugIndex) {
  for (int i = plugIndex; i < 4 - 1; ++i) {
      prioPlug[i] = prioPlug[i + 1];
    }
  prioPlug[4 - 1] = 0;  // Set the last element to 0 (or any default value)
    
}

void controlRelays(int plugValue) {
  Serial.print(plugValue);
  welcomescreen();
  // Turn off all relays
  Serial.println("Relay LOW");

  RLAS = false;
  RLBS = false;
  RLCS = false;
  RLDS = false;

  digitalWrite(RLA, LOW);
  digitalWrite(RLB, LOW);
  digitalWrite(RLC, LOW);
  digitalWrite(RLD, LOW);

  // Turn on the corresponding relay based on plugValue
  switch (plugValue) {
    case 15:
      digitalWrite(RLA, HIGH);
      Serial.println("Relay High A");
      RLAS = true;
      break;
    case 16:
      digitalWrite(RLB, HIGH);
      Serial.println("Relay High B");
      RLBS = true;
      break;
    case 17:
      digitalWrite(RLC, HIGH);
      RLCS = true;
      Serial.println("Relay High C");
      break;
    case 18:
      digitalWrite(RLD, HIGH);
      RLDS = true;
      Serial.println("Relay High D");
      break;
    case -1:
      break;
    // Add cases for plug3 and plug4 if needed
  }
  welcomescreen();
}

void drawButtonC(String label, int x, int y) {

  // Input: label (Label of buttons)
  //        int x (position of button x axis)
  //        int y (position of button y axis)
  // Output: None
  // Adjustable Variables: buttonWidth (Desired button width in pixels)
  //                       buttonHeight (Desired button height in pixels)
  // Description: Draw a box representing a button at position x, y. Each button will
  //              have a solid navy background and text will be in white.

  int buttonWidth = 62;
  int buttonHeight = 40;
  
  // Draw button background
  tft.drawRect(x + 1, y + 1, buttonWidth - 2, buttonHeight - 2, ILI9341_WHITE);
  tft.fillRect(x + 2, y + 2, buttonWidth, buttonHeight, ILI9341_GREEN);

  // Draw button label
  tft.setCursor(x + 10, y + 18);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.print(label);
}

void plugStat (int plugNum) {
  FillScreenBlank();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(30, 65);
  tft.print("Plug Status");
}
