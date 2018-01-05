#ifndef AUDIO_LIB_H
#define AUDIO_LIB_H

#include <math.h>

float MajorScale[] = {0,2,4,5,7,9,11};
#define RAND_FLOAT ((float)rand() / (float)RAND_MAX)
#define RAND_INT(Lo, Hi) (Lo + (rand() % (Hi - Lo)))
#define RAND_RANGE(Lo, Hi) (Lo+RAND_FLOAT*(Hi-Lo))

#define ARRAY_SIZE(Array) (sizeof(Array) / sizeof(*Array))
#define ARRAY_END(Array) (Array + ARRAY_SIZE(Array))
#define RANDOM_ITEM(Array) (Array[(int)floor(RAND_FLOAT * ARRAY_SIZE(Array))])

const double TAU = M_PI * 2;

float TransposeRatio(float Semitones);
float MIDIToFreq(float MIDINote);
float RandomFloat();
float RandomRange(float Low, float High);
float Clamp(float x, float lowerlimit, float upperlimit);
float Lerp(float From, float To, float X);

#endif // AUDIO_LIB_H
