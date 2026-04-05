#pragma once    

#define NUM_MICS 4 // mics in array
#define NUM_SAMPLES 16 // packet of samples to get avereage amplitude reading
#define C 0.001f // constant defined to avoid division by zero

// analog pins for individual mics
#define mic1 A3 
#define mic2 A4
#define mic3 A1
#define mic4 A2

// variables for threshold and confidence of sound impulse
#define THRESHOLD 300
#define CONFIDENCE 0.30f

extern const int mics[NUM_MICS];

void sampleMics(const int mics[], long acc[], int dcOffset);
void avgCalc(long amplitudes[], long acc[], int& totalAmp);
float calculateAngle(long amplitudes[]);