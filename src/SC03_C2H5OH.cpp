#include "SC03_C2H5OH.h"

SC03_C2H5OH::SC03_C2H5OH(uint8_t rxPin, uint8_t txPin) 
  : _serial(rxPin, txPin) {
  _baseline = 0;
  _calibrated = false;
}

void SC03_C2H5OH::begin() {
  _serial.begin(9600);
  Serial.println("Initializing SC03_C2H5OH sensor");

  for (int i = 5; i > 0; i--) {
    Serial.println("Wait for " + String(i) + " seconds");
    delay(1000);
  }

  Serial.println("Flushing unstable sensor data");
  for (int i = 0; i < 5; i++) {
    while (_serial.available() >= 9) {
      for (int j = 0; j < 9; j++) _serial.read();
    }
  }

  while (_serial.available()) _serial.read();

  Serial.println("Sensor ready.");
}


bool SC03_C2H5OH::read() {
  if (_serial.available() < 9) return false;
  
  while (_serial.available() && _serial.peek() != 0xFF) {
    _serial.read();
  }
  
  if (_serial.available() < 9) return false;
  
  for(int i = 0; i < 9; i++) {
    _data[i] = _serial.read();
  }
  
  return true;
}

void SC03_C2H5OH::calibrate() {
  float sum = 0;
  int count = 0;

  Serial.println("Starting calibration. Please, ensure clean-air");

  for (int i = 0; i < 10; i++) {
    if (read()) {
      float raw = getRaw();
      sum += raw;
      count++;
    }
    delay(1000);
  }

  if (count > 0) {
    _baseline = sum / count;
    if (_baseline < 0) _baseline = 0;
    _calibrated = true;

    Serial.println("Calibration complete. Baseline = " + String(_baseline));
  } else {
    Serial.println("Calibration failed: No valid readings.");
    _calibrated = false;
  }
}


String SC03_C2H5OH::getHex() {
  String result;
  char buf[4];
  for (int i = 0; i < 9; i++) {
    sprintf(buf, "%02X ", _data[i]);
    result += buf;
  }
  return result;
}

float SC03_C2H5OH::getRaw() {
  return (_data[4] * 256 + _data[5]) / 10.0;
}

float SC03_C2H5OH::getValue() {
  float raw = getRaw();
  float value = _calibrated ? raw - _baseline : raw;
  return (value < 0) ? 0 : value;
}
