#ifndef SC03_C2H5OH_H
#define SC03_C2H5OH_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class SC03_C2H5OH {
public:
  SC03_C2H5OH(uint8_t rxPin, uint8_t txPin);
  
  void begin();
  bool read();
  void calibrate();
  String getHex();
  float getRaw();
  float getValue();

private:
  SoftwareSerial _serial;
  uint8_t _data[9];
  float _baseline;
  bool _calibrated;
};

#endif
