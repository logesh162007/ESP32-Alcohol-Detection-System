# ESP32 Alcohol Detection and Motor Control System

**Course:** Microprocessors and Microcontrollers

## Project overview
An educational ESP32 prototype that reads an MQ-3 alcohol-vapour sensor, displays readings and status on an OLED, indicates status with LEDs, and controls a DC motor through an L298N motor driver. A local Wi-Fi access point serves a simple live status dashboard.

## Hardware components
- ESP32 development board
- MQ-3 sensor (analog output)
- 0.96-inch I2C OLED (SSD1306, address 0x3C)
- Green LED and red LED with suitable current-limiting resistors
- L298N motor driver and compatible DC motor
- Push button
- Suitable power supplies and common ground

## Pin connections used in the sketch

| Function | ESP32 GPIO |
|---|---:|
| MQ-3 analog output | 34 |
| Green LED | 25 |
| Red LED | 26 |
| Push button | 18 |
| L298N IN1 | 19 |
| L298N IN2 | 23 |
| L298N ENA | 5 |
| OLED SDA | 21 |
| OLED SCL | 22 |

## Arduino libraries
Install these libraries using Arduino IDE Library Manager:
- Adafruit GFX Library
- Adafruit SSD1306

The `WiFi.h`, `WebServer.h`, and `Wire.h` libraries are supplied with the ESP32 Arduino core.

## Upload and use
1. Open `ESP32_Alcohol_Detection_System.ino` in Arduino IDE.
2. Select the correct ESP32 board and COM/serial port.
3. Install the libraries listed above.
4. Upload the sketch.
5. Open Serial Monitor at **115200 baud** to see the access point details and sensor readings.
6. Connect a phone or computer to the ESP32 access point named `MQ3-Monitor`.
7. Open the dashboard at the IP address printed in Serial Monitor (commonly `192.168.4.1`).

## Important limitations and safety
- This is a classroom demonstration prototype, not a certified breathalyser, vehicle safety device, or proof of a person's blood-alcohol level.
- MQ-3 readings depend on sensor warm-up, calibration, environment, supply voltage, and sensor module. The example thresholds (`500` and `450`) are demonstration values and must not be treated as calibrated alcohol concentration.
- Do not use this prototype to make real-world driving or safety decisions.
- Do not connect a motor directly to an ESP32 GPIO. Use a suitable motor driver and compatible power supply. Ensure the ESP32, sensor, and motor-driver control circuit share a common ground where required.
- Check the voltage/current ratings of every module before powering the circuit.
- The Wi-Fi password in the example is a demo password. Change it before sharing the project publicly if you use a different or private password. Never publish personal Wi-Fi credentials.

## Course-project learning outcomes
- ESP32 programming
- Analog sensor interfacing
- I2C OLED interfacing
- GPIO-based LED and button control
- DC motor control using an L298N driver
- Basic Wi-Fi access point and web-server dashboard
