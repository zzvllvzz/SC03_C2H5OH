#include "SC03_C2H5OH.h"

SC03_C2H5OH sensor(10, 11); // TX, RX

//Initialize the sensor
void setup() {
  Serial.begin(9600);
  sensor.begin();
  sensor.calibrate();
}

//Read the data from the sensor
void loop() {
  if (sensor.read()) {
    Serial.print("HEX: ");
    Serial.print(sensor.getHex());
    Serial.print(" | Raw: ");
    Serial.print(sensor.getRaw());
    Serial.print(" | Calibrated: ");
    Serial.println(sensor.getValue());
  }
  
  /*Calibrate the sensor by typing 'c' in the serial terminal
   if (Serial.read() == 'c') {
     sensor.calibrate();
   }
  */ 
  delay(1000);
}
