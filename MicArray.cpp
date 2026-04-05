#include "MicArray.h"
#include <Arduino.h>
#include <math.h>

const int mics[NUM_MICS] = {mic1, mic2, mic3, mic4}; // array for collection of mics


// Sample mics sequentially, 1 at a time
void sampleMics(const int mics[], long acc[], int dcOffset){
    // Reset accumulators once at the start
    for (int j = 0; j < NUM_MICS; ++j) {
        acc[j] = 0;
    }
    
    for (int i = 0; i < NUM_SAMPLES; ++i) { // for each sample
        for (int j = 0; j < NUM_MICS; ++j) { // get data from each mic
            int pin = mics[j]; // select mic pin
            int sample = analogRead(pin); // read voltage from mic pin
            int v = sample - dcOffset; // remove DC offset from reading

            if (v < 0) v = -v; // We only want positive amplitude
            acc[j] += v; // add sample to this mic's accumulation array
        }
    }
}
//-----------------------------------------------------------------------
// Convert accumulators to averages
void avgCalc(long amplitudes[], long acc[], int& totalAmp){
    totalAmp = 0;
    for (int j = 0; j < NUM_MICS; ++j) {
        amplitudes[j] = acc[j] / NUM_SAMPLES; // average total accumulated readings for each mic
        totalAmp += amplitudes[j]; // sum averaged amplitudes of all mics
    }
}
//-----------------------------------------------------------------------
// calculate angle of sound wave
float calculateAngle(long amplitudes[]){
  float angle;
  // calculate difference of x and y axis
    // division by total level plus constant removes absolute volume and ensures no division by 0
    float x = ((float)(amplitudes[3] - amplitudes[2]) / (amplitudes[3] + amplitudes[2] + C)); // determine nomalized left to right direction
    float y = ((float)(amplitudes[0] - amplitudes[1]) / (amplitudes[0] + amplitudes[1] + C)); // determine normalized front to back direction
    // calculate vector magintude for confidence
    float r = sqrt(x*x + y*y);   // 0 = no direction, 1 = strong direction

    // Threshold for confidence
    if ( r <= CONFIDENCE){
      return -1.0f; // not confident enough
    }
    angle = atan2(y, x) * 180/PI; // use arcTan to take in ratio of length of y & x and return angle (*180/PI for degrees)      
     
    // print each mic amplitude, angle of sound source and dcOffset
    Serial.print("Mic1: "); Serial.println(amplitudes[0]);
    Serial.print("Mic2: "); Serial.println(amplitudes[1]);
    Serial.print("Mic3: "); Serial.println(amplitudes[2]);
    Serial.print("Mic4: "); Serial.println(amplitudes[3]);
    Serial.print("Approx direction: "); Serial.println(angle);   
    Serial.print("Confidence r: "); Serial.println(r);
    Serial.print("X: "); Serial.println(x);
    Serial.print("Y: "); Serial.println(y);

    // call goToAngle to set motor
    return angle;
}