//This is code for an ESP32 using Arduino IDE v2.2.1
//This code will read a clipped ac voltage from a current transformer

const int analogPin = 34; // Define the analog input pin (A0 on ESP32)
const float referenceVoltage = 3.3; // Set the reference voltage for the ESP32 (3.3V)
  float maxVoltage = 0.0; // Variable to store the maximum voltage
  int CTcounter = 0; // Counter variable

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 bps
}

void loop() {
    int sensorValue = analogRead(analogPin); // Read analog input from pin A0
    float voltage = (sensorValue / 4095.0) * referenceVoltage; // Convert to voltage
    
    if (voltage > maxVoltage) {
      maxVoltage = voltage; // This should find the peak of the voltage waveform
    }
    

    CTcounter++;
    if (CTcounter == 1017) {
      CTcounter = 0;
      maxVoltage = 0.0; // If the voltage decreases, then this simple code won't update. Therefore maxVoltage will be reset and will find
			// the new maximum, (if there is one).
			// Since the period is 1/60Hz = 17ms, the counter will count 1017 times because 1017 increments at 167us will make
			// the code check every 10 cycles of the current transformer's waveform [17ms * 10 / .167ms = 1017]
    }
  

  Serial.print("Max Voltage: ");
  Serial.print(maxVoltage, 2); // Print maximum voltage with 2 decimal places
  Serial.println(" V");
  
  delayMicroseconds(167); // Gives a sampling frequency of 6KHz. (Well, almost, I haven't taken in consideration how long
			  // it takes for the code to execute).
}
