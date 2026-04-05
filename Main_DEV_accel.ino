#include "MicArray.h"
#include "StepperControl.h"
#include <Arduino.h>

#include "RF_communication.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//--------------GLOBAL VARIABLES-------------------------------------------
#define RELAY_PIN 6
long acc[NUM_MICS] = {0, 0, 0, 0}; // array of accumulated voltages
long amplitudes[NUM_MICS]; // array of microphone ampitudes

RF24 radio(7, 8);                 // CE and CSN lines
//const byte address[6] = "00001";  //device address
const byte MAIN_ADDR[6] = "00001";
//const byte USER_ADDR[6] = "00002";
bool flashState = 0;

int state_angle = 0;

int dcOffset = 512;   // bias at VCC/2 (1023/2) 
int totalAmp = 0; // total amplitude for threshold

bool alertState = false; // initialize alertState to false

//--------------Flashlight helpers------------------------------------------
void initSpotlight(){
  pinMode(RELAY_PIN, OUTPUT); // set pin for spotlight operation
  digitalWrite(RELAY_PIN, HIGH); // initiallize spotlight off
}


//blocking func for AccelStepper.h
int deltaAngleBlocking(int angle) {
  // Calculate target position in steps
  long targetSteps = (long)(angle * STEPS_PER_DEG);
  
  // Get current position from stepper
  long currentSteps = stepper.currentPosition();
  
  // Calculate delta
  long delta = targetSteps - currentSteps;
  
  // Find shortest rotation path (handle wraparound)
  if (delta > (STEPS_PER_REV / 2)) delta -= STEPS_PER_REV;
  if (delta < -(STEPS_PER_REV / 2)) delta += STEPS_PER_REV;
  
  Serial.print("\tAngle: ");
  Serial.print(angle);
  Serial.print("\tDelta: ");
  Serial.println(delta);
  

  long newTarget = currentSteps + delta;
  stepper.runToNewPosition(newTarget);  // Blocks until complete
  // Return current angle if no movement needed
  return state_angle;
}


// function for turning off and on spotlight
void setSpotlight(bool on){
  if (on){
    digitalWrite(RELAY_PIN, HIGH); // turn on light
  }
  else{
    digitalWrite(RELAY_PIN, LOW); // turn off light
  }
}
///////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(2000); // give system time to initialize before running setup
  Serial.begin(9600); // initialize serial communication with 9600 baud rate
  initStepper(); // initialize stepper motor
  initSpotlight();  // initialize spotlight
  setHome(); // spotlight finds home location of photoresistor

  radio.begin();            //radio comm init
  radio.openWritingPipe(MAIN_ADDR);
  radio.openReadingPipe(0, MAIN_ADDR);
  radio.setPALevel(RF24_PA_MIN);
  
}

void loop() {
    //Sample mic array for impulse
    sampleMics(mics, acc, dcOffset);
    //Compute averages and average amplitudes
    avgCalc(amplitudes, acc, totalAmp);
    // If total amplitude of sound at mics is above the threshold then set True
    bool aboveThreshold = (totalAmp > THRESHOLD);

    // =====================================================
    //                        ALERT BLOCK
    // =====================================================
    if (!alertState && aboveThreshold) {
        // calculate angle of sound source and confidence
        float angle = calculateAngle(amplitudes);
        // if confident in direction
        if (angle != -1.0f) {
            // set alert state to true, turn on spotlight and go to the angle
            alertState = true;
            Serial.println("ALERT!");
            setSpotlight(true);
            goToAngle(angle);

            state_angle = angle;

            char txBuf[10];
            char rxBuf[10];
            memset(rxBuf, 0, 10);
            memset(txBuf, 0, 10);

            int mode  = 2;
            int flash = 1;
            int ang   = (int)angle;
            

            // FIRST PACKET to USER
            sprintf(txBuf, "%d:%d:%d", mode, flash, ang);

            Serial.print("MAIN FIRST TX:");
            Serial.print(txBuf);

            // Send first ping and wait for reply
            if (!main_send_receive(txBuf, rxBuf, 100)) {
                Serial.print("MAIN did NOT get first reply — aborting.");
                return;
            }

            // Parse USER reply
            sscanf(rxBuf, "%d:%d:%d", &mode, &flash, &ang);

            Serial.print("MAIN FIRST RX:");
            Serial.print(rxBuf);


            //control is passed to USER until mode is not 3
            while (mode != 3) {
              stepper.run();

                // --- Update spotlight / motor from USER ---
                setSpotlight(flash == 1);

                if (mode == 2) {
                    //deltaAngle(ang);
                    state_angle = deltaAngleBlocking(-ang); 

                }

                // MAIN prepares next TX packet
                sprintf(txBuf, "%d:%d:%d", mode, flash, ang);

                // Ping-Pong: MAIN -> USER   USER -> MAIN
                if (!main_send_receive(txBuf, rxBuf, 100)) {
                    Serial.print("PING TIMEOUT, trying again...\n");
                    continue;
                }

                // Parse USER new data
                sscanf(rxBuf, "%d:%d:%d", &mode, &flash, &ang);

                Serial.print("MAIN RX PONG <- ");
                Serial.print(rxBuf);
                Serial.print('\n');
                delay(5);
            }

            // =====================================================
            //             USER SENT MODE = 3 → RESET SYSTEM
            // =====================================================
            Serial.println("USER EXITED (MODE 3). RESET MAIN.");

            // reset alert state, turn light off and reset motor to home
            alertState = false;
            setSpotlight(false);
            state_angle = 0;
            Serial.println("Alert Reset");
            resetMotor();

            return;
        }
    }
}