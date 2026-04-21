# SC03_C2H5OH

Arduino library for the **SC03-C2H5OH** electrochemical ethanol (alcohol) gas
sensor by Shenzhen Shenchen Technology. The sensor streams blood-alcohol
concentration in mg/100 mL over UART (9600 8N1, 9-byte frames); this library
handles frame alignment, baseline calibration, and value conversion behind a
simple API.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![Version](https://img.shields.io/badge/version-1.0.2-blue)
![Architecture](https://img.shields.io/badge/arch-avr-green)
![Category](https://img.shields.io/badge/category-Sensors-lightgrey)

## Table of Contents

- [Features](#features)
- [Sensor Specifications](#sensor-specifications)
- [Wiring](#wiring)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
- [Calibration](#calibration)
- [Data Frame Format](#data-frame-format)
- [Examples](#examples)
- [Datasheet](#datasheet)
- [License](#license)

## Features

- Reads the SC03-C2H5OH sensor over SoftwareSerial on any two digital pins
- Automatic frame alignment on the `0xFF` header byte
- Built-in clean-air baseline calibration
- Separate raw / calibrated / hex accessors for flexibility
- Small footprint, no external dependencies beyond the Arduino core

## Sensor Specifications

| Parameter              | Value                                     |
|------------------------|-------------------------------------------|
| Target gas             | Ethanol (C₂H₅OH)                          |
| Output                 | UART (3.3 V TTL levels), 9600 8N1         |
| Working voltage        | **3.7 V – 5.5 V** (per datasheet)         |
| Range                  | 0 – 500 mg/100 mL (blood alcohol concentration) |
| Resolution             | ≤ 0.1 mg/100 mL                           |
| Warm-up time           | ≤ 5 s                                     |
| Response / recovery    | ≤ 5 s / ≤ 10 s                            |
| Working temperature    | −20 °C to +50 °C                          |
| Module size            | 18.2 × 15.2 × 6.5 mm                      |

> **Note on 3.3 V supply.** The datasheet specifies a working voltage of
> 3.7 V – 5.5 V, but in practice the sensor has been tested across several
> microcontrollers at **3.3 V** with output values effectively identical to
> those obtained at 5 V. This is outside the official spec — use at your
> own discretion — but it works fine for low-power, battery-driven designs
> where a 3.3 V rail is all that is available.

## Wiring

Sensor pinout (from the datasheet):

| Sensor Pin | Name       | Connect to                         |
|------------|------------|------------------------------------|
| 1, 2, 7    | NC         | —                                  |
| 3          | GND        | Arduino GND                        |
| 4          | VCC        | 3.7 – 5.5 V (3.3 V also works — see note above) |
| 5          | UART RXD   | Arduino TX pin (0 – 3.3 V input)   |
| 6          | UART TXD   | Arduino RX pin (0 – 3.3 V output)  |

In the bundled `Basic.ino` example, the SoftwareSerial pins are:

| Arduino Pin | Role         | Sensor Pin      |
|-------------|--------------|-----------------|
| 10          | RX (to Arduino) | Pin 6 — TXD  |
| 11          | TX (to Arduino) | Pin 5 — RXD  |

The sensor's UART output is 3.3 V TTL, which reads cleanly as a HIGH on 5 V
Arduino boards without a level shifter. The library only reads from the
sensor, so the Arduino → sensor TX line is not actively driven.

## Installation

**Arduino IDE — Library Manager**

1. Open `Sketch > Include Library > Manage Libraries…`
2. Search for `SC03_C2H5OH`
3. Click **Install**

**Arduino IDE — manual (ZIP)**

1. Download this repository as a ZIP (`Code > Download ZIP` on GitHub)
2. In the Arduino IDE, choose `Sketch > Include Library > Add .ZIP Library…`
3. Select the downloaded file

## Quick Start

```cpp
#include "SC03_C2H5OH.h"

SC03_C2H5OH sensor(10, 11); // rxPin, txPin

void setup() {
  Serial.begin(9600);
  sensor.begin();
  sensor.calibrate(); // take a clean-air baseline
}

void loop() {
  if (sensor.read()) {
    Serial.print("HEX: ");           Serial.print(sensor.getHex());
    Serial.print(" | Raw: ");        Serial.print(sensor.getRaw());
    Serial.print(" | Calibrated: "); Serial.println(sensor.getValue());
  }
  delay(1000);
}
```

## API Reference

### Constructor

```cpp
SC03_C2H5OH(uint8_t rxPin, uint8_t txPin);
```
Creates a sensor instance using SoftwareSerial. `rxPin` is the Arduino pin
that receives data from the sensor's TXD (pin 6); `txPin` is the Arduino pin
that drives the sensor's RXD (pin 5).

### Methods

| Method             | Returns  | Description                                                              |
|--------------------|----------|--------------------------------------------------------------------------|
| `void begin()`     | —        | Opens the serial port, waits 5 s for warm-up, flushes early unstable data. |
| `bool read()`      | `bool`   | Attempts to read one 9-byte frame, aligning on the `0xFF` header. Returns `true` on success. |
| `void calibrate()` | —        | Averages 10 readings to establish a clean-air baseline. Call **once** in clean air. |
| `float getRaw()`   | `float`  | Last raw reading in **mg/100 mL** (blood alcohol concentration), before baseline subtraction. |
| `float getValue()` | `float`  | Calibrated reading (`raw − baseline`, clipped at 0). Equals `getRaw()` if `calibrate()` has not been called. |
| `String getHex()`  | `String` | The full 9-byte frame formatted as space-separated hex — useful for debugging. |

## Calibration

`calibrate()` takes ten readings one second apart and stores their average as
the zero baseline. `getValue()` then returns the difference between the
current reading and that baseline, clipped to zero.

**Call `calibrate()` in clean ambient air**, after the 5-second warm-up, for
meaningful results. Until it has been called, `getValue()` returns the same
value as `getRaw()`.

## Data Frame Format

The sensor is active-upload by default, emitting one 9-byte frame every
second:

| Byte | Name                   | Example |
|------|------------------------|---------|
| 0    | Start                  | `0xFF`  |
| 1    | Gas name (C₂H₅OH)      | `0x17`  |
| 2    | Unit (mg/100 mL)       | `0x00`  |
| 3    | Decimal places         | `0x00`  |
| 4    | Concentration — high   | `0x00`  |
| 5    | Concentration — low    | `0x25`  |
| 6    | Full range — high      | `0x00`  |
| 7    | Full range — low       | `0x00`  |
| 8    | Checksum               | `0x25`  |

Concentration in mg/100 mL is computed as:

```
concentration = (data[4] << 8 | data[5]) / 10.0
```

`read()` aligns to the header byte, so partial frames received at startup are
automatically skipped. The checksum byte is exposed via `getHex()` for
inspection but is not currently verified by the library.

### Blood alcohol equivalences

From the datasheet:

```
1 mg/100 mL  =  0.001 ‰ BAC
             =  2.369 PPM exhaled ethanol
             =  0.0044 mg/L exhaled ethanol
```

## Examples

See [`examples/Basic`](examples/Basic) for a minimal sketch that initialises
the sensor, runs calibration, and prints raw, calibrated, and hex values
every second.

## Datasheet

The manufacturer datasheet is included in
[`extras/SC03-C2H5OH datasheet.pdf`](extras/SC03-C2H5OH%20datasheet.pdf).

## License

Released under the [MIT License](LICENSE).
