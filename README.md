# Wireless Acoustic Sensing Array for Security Detection
### SonicGuard | ENGR214, San Francisco State University, Fall 2025

## Authors
- Nikita Volkov — [github.com/spark1e](https://github.com/spark1e)
- Matt Carr — mcarr7@sfsu.edu

Real-time 2-node embedded security system using a 4-microphone array 
for sound localization with wireless remote control.

## System Architecture
<img width="1013" height="444" alt="image" src="https://github.com/user-attachments/assets/4a8b4e47-6d84-4179-9602-249e51362605" />

**Main Node (UNO1)** — detects sound, calculates angle, aims motorized spotlight  
**User Node (UNO2)** — remote control via RF (LCD, joystick, touch sensor)

## Demo
Sound source at 60° → System calculated 54.49° (confidence: 0.95)

## Physical Design
- 3D printed cross-beam chassis with acoustic baffles for mic isolation
- 4 microphones mounted on steel rods at 90° intervals
- Motorized flashlight mounted on stepper motor (360° rotation)
- Photoresistor-based automatic homing on startup
- Custom 3D printed enclosure for user control center

## Hardware
- 2x Arduino Uno R3
- 4-channel analog microphone array (MAX4466, A1-A4)
- nRF24L01 RF modules (bidirectional, tested at 1m and 10m)
- Stepper motor (28BYJ-48) with ULN2003 driver
- Relay module for flashlight control
- I2C LCD display, joystick, touch sensor (TTP223B)
- USB PSU for stable 5V/high-current supply

## Circuit Schematics
<img width="1174" height="644" alt="image" src="https://github.com/user-attachments/assets/0f4178b3-a827-4e5f-b9f0-40cbd7d091ea" />


## Key Technical Details
- Amplitude-based sound localization using atan2()
- 16-sample averaging buffer to reduce noise jitter
- Confidence threshold filtering (r > 0.30) to reject weak signals
- Bidirectional RF with ACK, timeout handling and auto-reconnect
- Finite state machine: auto mode ↔ manual control
- Communication protocol: MODE:FLASH:ANGLE (e.g. 2:1:-155)
- Modular C++ design (MicArray, StepperControl, RF_communication)

## Software Flow
<img width="1017" height="630" alt="image" src="https://github.com/user-attachments/assets/fbf76d24-c4a8-4220-8cdc-b4463d647b84" />


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
- AccelStepper
- LiquidCrystal I2C
- RF24

## Course
ENGR214 — San Francisco State University, Fall 2025
