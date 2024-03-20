#include <Arduino.h>

const int analogInPinOne = 6; // You can change this pin according to your wiring
const int analogInPinTwo = 7; // Add more analog pins if needed
const int analogInPinThree = 4;
const int analogInPinFour = 5;

float maxVoltageOne = 0.0;
float maxVoltageTwo = 0.0;
float maxVoltageThree = 0.0;
float maxVoltageFour = 0.0;

int resetMaxCounter = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  analogSetAttenuation(ADC_0db);
}

void loop() {
	
  // Read the analog inputs
  int sensorValueOne = analogRead(analogInPinOne);
  delay(5);
  int sensorValueTwo = analogRead(analogInPinTwo);
  delay(5);
  int sensorValueThree = analogRead(analogInPinThree);
  delay(5);
  int sensorValueFour = analogRead(analogInPinFour);
  delay(5);

  float voltageOne = sensorValueOne * (3.3 / 4095.0);
  float voltageTwo = sensorValueTwo * (3.3 / 4095.0);
  float voltageThree = sensorValueThree * (3.3 / 4095.0);
  float voltageFour = sensorValueFour * (3.3 / 4095.0);

  if (voltageOne > maxVoltageOne) {
    maxVoltageOne = voltageOne;
  }

  if (voltageTwo > maxVoltageTwo) {
    maxVoltageTwo = voltageTwo;
  }

  if (voltageThree > maxVoltageThree) {
    maxVoltageThree = voltageThree;
  }

  if (voltageFour > maxVoltageFour) {
    maxVoltageFour = voltageFour;
  }

  // Print the raw ADC values and voltages
  //Serial.print("ADC Value 1: ");
  //Serial.print(sensorValueOne);
  /* Serial.print("\n Voltage 1: ");
  Serial.println(voltageOne);
  Serial.print(" Max Voltage 1: ");
  Serial.println(maxVoltageOne);*/

  //Serial.print("ADC Value 2: ");
  //Serial.print(sensorValueTwo);
  Serial.print("\n Voltage 2: ");
  Serial.println(voltageTwo);
  Serial.print(" Max Voltage 2: ");
  Serial.println(maxVoltageTwo);

  //Serial.print("ADC Value 3: ");
  //Serial.print(sensorValueThree);
  /* Serial.print("\n Voltage 3: ");
  Serial.println(voltageThree);
  Serial.print(" Max Voltage 3: ");
  Serial.println(maxVoltageThree); */


  //Serial.print("ADC Value 4: ");
  //Serial.print(sensorValueFour);
  /*Serial.print("\n Voltage 4: ");
  Serial.println(voltageFour);
  Serial.print(" Max Voltage 4: ");
  Serial.println(maxVoltageFour);*/

  if (resetMaxCounter > 1000) {
    maxVoltageOne = 0.0;
    maxVoltageTwo = 0.0;
    maxVoltageThree = 0.0;
    maxVoltageFour = 0.0;
    resetMaxCounter = 0;
    Serial.print("\n?\n?\n?\n?\nMAX HAS BEEN RESET\n?\n?\n?\n?\n?");
    delay(3000);
  }  

  resetMaxCounter++;
}