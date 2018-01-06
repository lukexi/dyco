#include "audio-lib.h"
#include <stdlib.h>

float TransposeRatio(float Semitones) {
    return pow(2, Semitones/12);
}

float MIDIToFreq(float MIDINote) {
    const float A440Note = 69;
    return 440 * TransposeRatio(MIDINote - A440Note);
}

float RandomFloat() {
    return (float)rand() / (float)RAND_MAX;
}

float RandomRange(float Low, float High) {
    const float Range = High-Low;
    return Low + Range * RandomFloat();
}

float Clamp(float x, float lowerlimit, float upperlimit) {
  if (x < lowerlimit)
    x = lowerlimit;
  if (x > upperlimit)
    x = upperlimit;
  return x;
}

float Lerp(float From, float To, float X) {
    return From + ((To - From) * Clamp(X, 0, 1));
}
