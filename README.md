# wireless-acoustic-security-array
Real-time sound localization system with 4-mic array, stepper motor control, and wireless RF communication | Arduino C++

# Wireless Acoustic Sensing Array for Security Detection

Real-time 2-node embedded security system using a 4-microphone 
array for sound localization with wireless remote control.

## Demo
Sound source at 60° → System calculated 54.49° (confidence: 0.95)

## System Architecture
**Main Node** — detects sound, calculates angle, aims spotlight  
**User Node** — remote control via RF (LCD, joystick, touch sensor)

## Hardware
- 2x Arduino Uno R3
- 4-channel analog microphone array (A1-A4)
- nRF24L01 RF modules (bidirectional, tested at 1m and 10m)
- Stepper motor with photoresistor-based homing
- I2C LCD display, joystick, touch sensor

## Key Technical Details
- Amplitude-based sound localization using atan2()
- 16-sample averaging buffer to reduce noise jitter
- Confidence threshold filtering (r > 0.30) to reject weak signals
- Bidirectional RF with ACK, timeout handling and auto-reconnect
- Finite state machine: auto mode ↔ manual control
- Modular C++ design (MicArray, StepperControl, RF_communication)

## Results
| Trial | Real Angle | Calculated | Error |
|-------|-----------|------------|-------|
| 1     | 45°       | 48°        | 3°    |
| 2     | 180°      | 210°       | 30°   |
| 3     | -45°      | -50°       | 5°    |
| 4     | 90°       | 81°        | 9°    |
| 5     | -180°     | -173°      | 7°    |

- Angular accuracy within 10° in 4 of 5 trials
- 3/3 successful detections in live demonstration
- Stable RF communication at 1m and 10m

## Dependencies
- [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/)
- [LiquidCrystal I2C](https://github.com/johnrickman/LiquidCrystal_I2C)
- [RF24](https://github.com/nRF24/RF24)

## Authors
- Nikita Volkov — [github.com/spark1e](https://github.com/spark1e)
- Matt Carr — mcarr7@sfsu.edu

## Course
ENGR214 — San Francisco State University, Fall 2025
