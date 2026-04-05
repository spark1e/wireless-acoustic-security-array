#include "StepperControl.h"
#include <Arduino.h>

void setSpotlight(bool on);

AccelStepper stepper(AccelStepper::FULL4WIRE, 2, 4, 3, 5);

#define PHOTO_PIN A0
long numSteps = 0; // number of motor steps
long home = 0;

void initStepper(){
  stepper.setMaxSpeed(1500.0); // initialze step motor speed - theoretical up to 4k
  stepper.setAcceleration(800.0); // initialze step
}

// convert degrees to motor steps and set motor
void goToAngle(float angle){
  numSteps = (long)(angle * STEPS_PER_DEG);
  stepper.runToNewPosition(numSteps);
}

void resetMotor(){
  stepper.runToNewPosition(home);
}

// sets the home position of the motor
void setHome(){
  float maxAngle = 0;
  int minVal = 1024;
  int minPos = -1;

  setSpotlight(true);     // turn on spotlight
  delay(100);

  // sweep for 500 steps starting at -90 degrees to find max reading from photoresistor
  goToAngle(-90);
  stepper.moveTo(500);

  Serial.println(minPos);
  // while we have steps left from the 500, keep running and sampling voltages
  
  while (stepper.distanceToGo() != 0){
    stepper.run();
    int val = analogRead(PHOTO_PIN);
    Serial.println(val);
    // replace maxVal and update maxPos for the highest voltage reading
    if (val < minVal){
      minVal = val;
      minPos = stepper.currentPosition();
    }
  }

  // set the home position and turn light off
  stepper.runToNewPosition(minPos);
  stepper.setCurrentPosition(0);
  home = 0;
  setSpotlight(false);
}