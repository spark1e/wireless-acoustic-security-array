#pragma once
#include <AccelStepper.h>

// steps per revolution for step motor and convert to degress
const float STEPS_PER_REV = 2048.0;
const float STEPS_PER_DEG = STEPS_PER_REV / 360.0;

extern AccelStepper stepper;

void goToAngle(float angle);
void initStepper();
void resetMotor();
void setHome();

