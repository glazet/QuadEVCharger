/*
***************************************************************************************
* Project Name: Smart Quad EV Charger                                                 *
* Team Name: SQEV                                                                     *
* Affiliated Institution: Sonoma State University Senior Design 2024                  *
* Team Members: Calvin Pereira, Trent Glaze, Django Demetri                           *
* IDE: Arduino IDE, V2.3.2                                                            *
* Github: https://github.com/glazet/QuadEVCharger                                     *
* Website: https://smartquadevcharger.weebly.com/                                     *
* Abstract:                                                                           *
*   The Smart Quad EV Charger project aims to address the increasing demand for       *
*   Electric Vehicle (EV) charging infrastructure in the face of surging EV ownership.*
*   Designed as a Level-2 Electric Vehicle Supply Equipment (EVSE), this innovation   *
*   accommodates up to four standard EVs simultaneously, optimizing charging          *
*   capabilities within constrained parking spaces. Utilizing SAE communication       *
*   protocols such as control pilot signals, the charger integrates an algorithm      *
*   that dynamically manages charging sessions to provide charge to each plugged in   *
*   EV. Rigorous testing, covering parameters such as control pilot voltage and       *
*   frequency tolerances, along with switching device control, verifies the           *
*   successful implementation of essential engineering requirements. This project     *
*   offers an innovative solution to the evolving landscape of EV charging needs.     *
***************************************************************************************
*/


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

#define TFT_DC 35                                            // DC
#define TFT_CS 8                                             // CS
#define TFT_MISO -1                                          // Not Used
#define TFT_CLK 12                                           // CLK
#define TFT_RST 11                                           // RST
#define TFT_MOSI 37                                          // MOSI

/////////////////////////////////////////////////////////////////////////////////////
// RFPID Pin Definition (NFC Reader)
/////////////////////////////////////////////////////////////////////////////////////

#define RFID_SS_PIN 10                                        // SDA
#define RFID_RST_PIN 14                                       // RST
#define RFID_SCK_PIN 36                                       // SCK
#define RFID_MOSI_PIN 38                                      // MOSI
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

int buttonNum = -1;                                            // Used in function checkButtonPress returns button number
int button[4] = {1, 2, 20, 19};                               // Button Pins

int LEDG = 0;                                                  // RED LED
int LEDR = 45;                                                 // Green LED
int LEDF = 3;                                                  // Fan LED

int CdutyCycle = 169;                                          // Duty cycle
int NdutyCycle = 0;                                            // No duty cycle
int adcValue;
int pilotFB;

const int numSamples = 500;                                   // Number of samples to take used for pilotRead
float peakVoltage = 0.0;                                      // Used to store Peak Voltage
int numMeasurements = 10;                                     // Number of measurements to average
float totalPeakVoltage = 0.0;                                 // Peak voltage after sampling
const float Vref = 0.95;                                      // 950mV

float CTtotalPeakVoltage[4];
float CTpeakVoltage[4] = {0.0, 0.0, 0.0, 0.0};
float CTadcValue = 0.0;
float CTfinalVoltage[4] = {0.0, 0.0, 0.0, 0.0}; // Array to hold the average voltage for each pin
const int CTPins[] = {15, 16, 17, 18}; //the pin numbers

int prioPlug[4] = {0, 0, 0, 0};                               // Prio Array
String nfcInfo[4] = {"", "", "", ""};                         // NFC array used to store NFC ID

//Plug Arrays {Car Status, Relay Status, priority}
//Default state -1
// Car Status: 
// Relay Status: 0, 1 (0 = OFF, 1 = ON)
// Priority: 0, 1, 2, 3 (0 = Charging)

int plugSA[4] = {12, -1, -1, -1}; 
int plugSB[4] = {12, -1, -1, -1}; 
int plugSC[4] = {12, -1, -1, -1}; 
int plugSD[4] = {12, -1, -1, -1}; 


const int plug[4] = {6, 7, 4, 5};
const int pwm[4] = {47, 48, 46, 9};
const int RL[4] = {42, 41, 40, 39};




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

int readNFC();                                                // Function used to read NFC waits 10 seconds
String getTagID(byte *buffer, byte bufferSize);               // Converting ID into a String
void welcomescreen();                                         // Main Screen
void drawButton(String label, int x, int y);                  // Used to create boxes on LCD representing buttons
bool TapCardScreen(int plugName);                             // Prompt users to tap their NFC Card
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

  analogReadResolution(12); // Set ADC resolution to 12 bits
  // Set attenuation to 0 dB
  analogSetAttenuation(ADC_0db);


  // Setting corrisponding pins to output or input
  pinMode(LEDG, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(button[0], INPUT);
  pinMode(button[1], INPUT);
  pinMode(button[2], INPUT);
  pinMode(button[3], INPUT);

  pinMode(RL[0], OUTPUT);
  pinMode(RL[1], OUTPUT);
  pinMode(RL[2], OUTPUT);
  pinMode(RL[3], OUTPUT);

  pinMode(pwm[0], OUTPUT);
  pinMode(pwm[1], OUTPUT);
  pinMode(pwm[2], OUTPUT);
  pinMode(pwm[3], OUTPUT);

  pinMode(plug[0], INPUT);
  pinMode(plug[1], INPUT);
  pinMode(plug[2], INPUT);
  pinMode(plug[3], INPUT);

  

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

  controlRelays(-1);

}

/////////////////////////////////////////////////////////////////////////////////////
// Void Loop
/////////////////////////////////////////////////////////////////////////////////////

void loop() {
  
  unsigned long ChTime = millis() + 15000;
  while (millis() < ChTime) {
    unsigned long GetTime = millis() + 5000;
    while (millis () < GetTime) {
      getTime();
      iswifiUP();
    }
    
      
    plugSA[0] = pilotRead(plug[0]);
    plugSB[0] = pilotRead(plug[1]);
    plugSC[0] = pilotRead(plug[2]);
    plugSD[0] = pilotRead(plug[3]);
    
    updatePrio();

    checkButtonPress();
    
    
    if (buttonNum != -1) {
      plugStat(buttonNum, ChTime);
      
      welcomescreen();
      buttonNum = -1;
    }
    

  
  }
  

  offLED();
  
  
  delay(5);

  controlRelays(prioPlug[0]);
  prioPlug[0] = 0;
  for (int i = 0; i < 4 - 1; ++i) {
    prioPlug[i] = prioPlug[i + 1];
  }
  prioPlug[4 - 1] = 0;
  updatePrio();

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
    //if (checkButtonPress() == 4) {                                           // Check if exit button is pressed
    //  return 2;                                                              // If exit button is pressed return 2
    //}
  }
  return 3;                                                                   // Return false if tag is not read
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

  tft.fillRect(0, 0, tft.width(), tft.height(), ILI9341_WHITE);             // Draw rectangle to clear screen
  

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
  tft.setTextColor(ILI9341_BLACK);
  tft.print("Welcome EV Owner");

  // Draw buttons
  drawButton("Plug 1", 20, 185);
  drawButton("Plug 2", 97, 185);
  drawButton("Plug 3", 173, 185);
  drawButton("Plug 4", 250, 185);


  if (plugSA[1] == 1) {
    drawButtonC("Plug 1", 20, 185);
  }
  else if (plugSB[1] == 1) {
    drawButtonC("Plug 2", 97, 185);
  }
  else if (plugSC[1] == 1) {
    drawButtonC("Plug 3", 173, 185);
  }
  else if (plugSD[1] == 1) {
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
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(75,75);
  tft.setTextSize(2);
  tft.print("Please Tap Card");
  tft.setCursor(230, 10);
  
  

  if (plugName == plug[0]) {
    tft.print("Plug: 1");
  }

  if (plugName == plug[1]) {
    tft.print("Plug: 2");
  }
  if (plugName == plug[2]) {
    tft.print("Plug: 3");
  }
  if (plugName == plug[3]) {
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
    tft.setTextColor(ILI9341_BLACK);
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
    

    if (plugName == plug[0] && nfcInfo[0].isEmpty()) {
      nfcInfo[0] = tagID;
    }

    if (plugName == plug[1]  && nfcInfo[1].isEmpty()) {
      nfcInfo[1] = tagID;
    }
    if (plugName == plug[2]  && nfcInfo[2].isEmpty()) {
      nfcInfo[2] = tagID;
    }
    if (plugName == plug[3]  && nfcInfo[3].isEmpty()) {
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
    //tft.fillScreen(ILI9341_WHITE);
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


  
  if (digitalRead(button[0]) == LOW) {
      buttonNum = 1;
  }
  

  /*

  else if (digitalRead(button[1] == LOW) {
      buttonNum = 2;
  }
  

  else if (digitalRead(button[2]) == LOW) {
      buttonNum = 3;
  }
  
  
  */
  else if (digitalRead(button[3]) == LOW) {
      buttonNum = 4;    
  }
  
  
  
  else {
    //buttonNum = -1;
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
  if (LEDcolor == "F") {
    digitalWrite(LEDF, HIGH);
    delay(1000);
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
  //digitalWrite(LEDF, LOW);
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
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(20, 75);
  tft.print("Beginning Initial Startup");
  delay(5);

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
      tft.setCursor(80, 75);
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
    delay(5);

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

  // Input: None
  // Output: None
  // Adjustable Variables: None
  // Description: Grabs most recent time from libary and outputs it to the screen


  timeClient.update();
  String currentTime = timeClient.getFormattedTime();
  int index = currentTime.indexOf(":");
  String hoursMinutes = currentTime.substring(0, index + 3);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  
  tft.fillRect(0, 0, 150, 40, ILI9341_WHITE);
  tft.print(hoursMinutes);

}

/////////////////////////////////////////////////////////////////////////////////////
// pilotRead
/////////////////////////////////////////////////////////////////////////////////////

int pilotRead(int plugNum) {

  // Input: plugNum
  // Output: Pilot Voltage
  // Adjustable Variables: ADC read Voltages, numMeasurements, numSamples
  // Description: Reades voltages at each plug and performs a series of functions
  //              corrisponding to voltage read.


  bool isPlugNumInArray = false;  // Check if plug is already in the array
  int plugIndex = -1;             // To store the index of plugNum in the array
  int nfcIndex = 0;               // Store NFC ID in array 0 = plug1, 1 = plug2, ....
  bool nfcRead = false;           // Check if NFC is read
  float voltage = 0;              // ADC Read Voltage
  int* plugRCheck;

/////////////////////////////////////////////////////////////////////////////////////
  
  // Checks if plugNum is in array, Starts at element 0 and checks if plugNum is equal
  // to the element. If plugNum is equal to the element set isPlugNumInArray to true.
  // As well as set plugIndex to I where it remembers which element plugNum is in used
  // to remove plug from prioPlug. 
  for (int i = 0; i < 4; ++i) {
    if (prioPlug[i] == plugNum) {
      isPlugNumInArray = true;
      plugIndex = i;
      break;
    }
  }

  // Switch case to store nfc ID in nfcIndex. nfcIndex[0] related to plug1...
  switch (plugNum) {
    case 6:
      nfcIndex = 0;
      plugRCheck = plugSA;
      break; 
    case 7:
      nfcIndex = 1;
      plugRCheck = plugSB;
      break;
    case 4:
      nfcIndex = 2;
      plugRCheck = plugSC;
      break;
    case 5:
      nfcIndex = 3;
      plugRCheck = plugSD;
      break;
  }


  // Check if nfc is read 
  if (!nfcInfo[nfcIndex].isEmpty()) {
    nfcRead = true;
  }

  /////////////////////////////////////////////////////////////////////////////////////

  // Read the voltage at plugNum, samples ADC specified by numMeasurements, Averages
  // the peak voltage specified by numSamples. Then converts ADC value into useable
  // voltage value. Where the reference voltage is 0.95mV

  totalPeakVoltage = 0.0;
  for (int j = 0; j < numMeasurements; j++) {
    peakVoltage = 0.0; // Reset peak voltage for each measurement
    // Sample the signal and find peak voltage
    for (int i = 0; i < numSamples; i++) {
      adcValue = analogRead(plugNum);
      voltage = adcValue * (Vref / 4095.0); // Convert ADC value to voltage using custom Vref
      if (voltage > peakVoltage) {
        peakVoltage = voltage;
      }
      delayMicroseconds(10); // Adjust delay as needed based on your sampling rate
    }
    totalPeakVoltage += peakVoltage; // Accumulate peak voltage for averaging
  }
  
  voltage = (totalPeakVoltage / numMeasurements) - 0.04; // Calculate average peak voltage


  /////////////////////////////////////////////////////////////////////////////////////

  // When pilot voltage is at 12V meaning no EV is plugged in. Ensure the plugNum is not
  // in prioPlug and ensure NFC element related to plug is clear.

  if (voltage > 0.81 && voltage < 0.9) {

    if (!isPlugNumInArray) {
      prioPlug[plugIndex] = 0;
      nfcInfo[nfcIndex].clear();
      formatPrio();
    }

    return 12;
  }

  /////////////////////////////////////////////////////////////////////////////////////

  // When pilot voltage is at 9V the EV is plugged in on standby. 
  // Ensure that the plugNum is not in prioPlug and ensure NFC element related to plug 
  // is not read. If plug is not in prioPlug and NFC element is empty promt user to 
  // tap card. Once card is tapped add plug to the prio list.

  else if (!nfcRead && !isPlugNumInArray && voltage > 0.69 && voltage < 0.80) {

    if (TapCardScreen(plugNum)) {
      addToPrio(plugNum);
    }

    else {
      welcomescreen();
      return -1;
    }
  
    welcomescreen();
    return 9;
  }

  /////////////////////////////////////////////////////////////////////////////////////

  // When pilot read is at 6V this represents that the EV is charging we will do nothing
  // in this case.

  else if (voltage > 0.55 && voltage < 0.62) {
    return 6;
  }

  /////////////////////////////////////////////////////////////////////////////////////

  // When pilot read is at 3V this represents the EV is charging with fan ventilation.
  // Turn on fan led to represent ventilation. This led stays on until voltage changes.

  else if (voltage > 0.49 && voltage < 0.52) {
    onLED("F");
    return 3;
  }

  /////////////////////////////////////////////////////////////////////////////////////

  // When pilot read is close to 0 this indicates an error in the system. Turn on the
  // Red led representing a faulty EVSE

   else if (voltage > 0.2 && voltage < 0.4) {
    return 0;
  }

  /////////////////////////////////////////////////////////////////////////////////////

  // When pilot read is close to 0 this indicates an error in the system. Turn on the
  // Red led representing a faulty EVSE

  else if (voltage > 0.03 && voltage < 0.02) {
    onLED("R");
    FillScreenBlank();
    tft.setCursor(90, 110);
    tft.setTextColor(ILI9341_RED);
    tft.print("ERROR");
    while (1);
  }

  else if (nfcRead && !isPlugNumInArray && plugRCheck[1] == 0) {
    addToPrio(plugNum);
  }

  /////////////////////////////////////////////////////////////////////////////////////

  // Otherwise return an unknown voltage

  else {
    return -1;
  }
  
}



/////////////////////////////////////////////////////////////////////////////////////
// addToPrio & formatPrio
/////////////////////////////////////////////////////////////////////////////////////
  // Input: plugNum
  // Output: None
  // Adjustable Variables: None
  // Description: Adds plugNum to priority, also ensures there are no spaces in
  //              the array.

void addToPrio (int plugNum) {
  for (int i = 0; i < 4; ++i) { // Corrected loop condition
    if (prioPlug[i] == 0) { // Corrected comparison operator from = to ==
      prioPlug[i] = plugNum;
      break; // Exit the loop after finding an empty slot
    }
}
}

void formatPrio() {
  for (int i = 0; i < 3; ++i) {
    if (prioPlug[i] == 0 && prioPlug[i + 1] != 0) {
      // Shift elements to the left if there's a zero between non-zero elements
      for (int j = i; j < 3; ++j) {
        prioPlug[j] = prioPlug[j + 1];
      }
      prioPlug[3] = 0; // Set the last element to zero after shifting
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////
// controlRelays
/////////////////////////////////////////////////////////////////////////////////////
  // Input: plugNum, time (total time charged)
  // Output: None
  // Adjustable Variables: buttonWidth (Desired button width in pixels)
  //                       buttonHeight (Desired button height in pixels)
  // Description: Displays the status of selected plug, Charge state, time charged,
  //              and power in kWh. If there are multiple cars plugged in this screen
  //              will print the priority posistion. 

void controlRelays(int plugValue) {
  
  welcomescreen();
  


  plugSA[1] = 0;
  plugSB[1] = 0;
  plugSC[1] = 0;
  plugSD[1] = 0;

  analogWrite(pwm[0], NdutyCycle);
  analogWrite(pwm[1], NdutyCycle);
  analogWrite(pwm[2], NdutyCycle);
  analogWrite(pwm[3], NdutyCycle);

  
  digitalWrite(RL[0], LOW);
  digitalWrite(RL[1], LOW);
  digitalWrite(RL[2], LOW);
  digitalWrite(RL[3], LOW);
  delay(2000);

  // Turn on the corresponding relay based on plugValue
  switch (plugValue) {
    case 6:
      analogWrite(pwm[0], CdutyCycle);
      digitalWrite(RL[0], HIGH);
      plugSA[1] = 1;
      break;
    case 7:
      analogWrite(pwm[1], CdutyCycle);
      digitalWrite(RL[1], HIGH);
      plugSB[1] = 1;
      break;
    case 4:
      analogWrite(pwm[2], CdutyCycle);
      digitalWrite(RL[2], HIGH);
      plugSC[1] = 1;
      break;
    case 5:
      analogWrite(pwm[3], CdutyCycle);
      digitalWrite(RL[3], HIGH);
      plugSD[1] = 1;
      break;
    case -1:
      break;
    default:
      break;
    // Add cases for plug3 and plug4 if needed
  }
  welcomescreen();
}

/////////////////////////////////////////////////////////////////////////////////////
// drawButtonC
/////////////////////////////////////////////////////////////////////////////////////

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

void updatePrio () {
  
  
  for (int i = 0; i < 4; i++) {
    
    if (prioPlug[i] == plug[0]) {
      plugSA[2] = i;
      
    }
    else if (prioPlug[i] == plug[1]) {
      plugSB[2] = i;
    }
    else if (prioPlug[i] == plug[2]) {
      plugSC[2] = i;
    }
    else if (prioPlug[i] == plug[3]) {
      plugSD[2] = i;
    }
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////
// plugStat
/////////////////////////////////////////////////////////////////////////////////////
  // Input: plugNum, time (total time charged)
  // Output: None
  // Adjustable Variables: buttonWidth (Desired button width in pixels)
  //                       buttonHeight (Desired button height in pixels)
  // Description: Displays the status of selected plug, Charge state, time charged,
  //              and power in kWh. If there are multiple cars plugged in this screen
  //              will print the priority posistion. 

void plugStat(int plugNum, int time) {
  
  unsigned long startTime = millis();
  bool exitPressed = false; 
  FillScreenBlank();
  getTime();
  drawButton("EXIT", 250, 160);
  
  // Used to capture current voltage
  int CTplace = -1;
  
  //Display this page for 10 seconds
  while (millis() - startTime < 10000) {
    
    // Check if button 4 is pressed then exit
    if (checkButtonPress() == -1) {
      break;
    
    }
    
    // Print Plug Status
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(90, 30);
    tft.print("Plug Status");
    
    
    // Print the plug Number
    tft.setCursor(240, 10);
    tft.print("Plug ");
    tft.print(plugNum);

    // Create a new array that is only used in this function that points
    // to the status array of selected plug
    int* currentPlugArray; // Pointer to current plug array

    // Determine which plug array to use based on plugNum
    switch (plugNum) {
       case 1:
        currentPlugArray = plugSA;
        CTplace = 0;
        break;
      case 2:
        currentPlugArray = plugSB;
         CTplace = 1;
         break;
       case 3:
         currentPlugArray = plugSC;
         CTplace = 2;
         break;
       case 4:
         currentPlugArray = plugSD;
         CTplace = 3;
         break;
       default:
         // Handle unexpected case
         return;
     }

    // If the relay status of the plug is set to 1 in array
     if (currentPlugArray[1] == 1) {
        // Print Charging car
        tft.setCursor(90, 110);
        tft.setTextColor(ILI9341_GREEN);
        tft.print("Charging Car");
        
        // Calculate remaining charging time in minutes
        int remainingMinutes = (time - millis() + startTime) / (1000 * 60);
        tft.setCursor(90, 140);
        tft.setTextColor(ILI9341_BLACK);
        //tft.print("Remaining Time: ");
        tft.print(remainingMinutes);
        tft.print(" minutes");

        // Call CTread to calculate power in KWh
        CTread();
        // Print power to screen
        tft.setCursor(90, 90);
        tft.setTextColor(ILI9341_BLACK);
        tft.print("KWH: ");
        tft.print(CTfinalVoltage[CTplace]);
        

     } else {
        // If relay status is set to 0 in array
        // Print Not charging
        tft.setCursor(90, 110);
        tft.setTextColor(ILI9341_RED);
        tft.print("Not Charging");
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(90, 60);
        // Print the Priority number
        tft.print("Priority: ");
        tft.print(currentPlugArray[2]);

     }      
   }
   delay(100);
  
  return;
}

/////////////////////////////////////////////////////////////////////////////////////
// CTread
/////////////////////////////////////////////////////////////////////////////////////
  // Input: None
  // Output: None
  // Adjustable Variables: 
  // Description: Cycles through each CTpins to read current (four votlages). Samples
  //              and averages the voltage. Then outputs the calculated power to an
  //              array.

void CTread() {
  for (int z = 0; z < 4; z++) {//for loop that cycles through each index of CTPins, aka, all four of the voltages we want in the code
    CTtotalPeakVoltage[z] = 0.0;
    for (int x = 0; x < numMeasurements; x++) {
      CTpeakVoltage[z] = 0.0;  // Reset peak voltage for each measurement
      // Sample the signal and find peak voltage
      for (int y = 0; y < numSamples; y++) {
        CTadcValue = analogRead(CTPins[z]); 
        float voltage = CTadcValue * (Vref / 4095.0); 
        if (voltage > CTpeakVoltage[z]) {
          CTpeakVoltage[z] = voltage;
        }
        delayMicroseconds(10);  // Adjust delay as needed based on your sampling rate
      }
      CTtotalPeakVoltage[z] += CTpeakVoltage[z];  // Accumulate peak voltage for averaging
    }
    CTfinalVoltage[z] = (CTtotalPeakVoltage[z] / numMeasurements + .01); // Calculate average peak voltage for each pin
  }
}



