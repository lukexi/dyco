/* http://yehar.com/blog/?p=121
Fast lowpass with resonance v2

resofreq = resonation frequency  (must be < SR/4)
amp = magnitude at the resonation frequency

Init:
fx = cos(2*pi*resofreq / samplerate)
c = 2-2*fx
r = (sqrt(2)*sqrt(-(fx-1)^3)+amp*(fx-1))/(amp*(fx-1))
pos = 0
speed = 0

Loop:
speed = speed + (input(t) - pos) * c
pos = pos + speed
speed = speed * r
output(t) = pos
*/

typedef struct {
     float Freq;
     float Res;
     float FX;
     float C;
     float R;
     float Pos;
     float Speed;
} lowpass_filter;

float Lowpass(
     lowpass_filter* State, int SampleRate,
     float Freq, float Res,
     float In)
{
     if (Freq != State->Freq || Res != State->Res) {
          float FX = cos(2*M_PI*Freq / (float)SampleRate);
          float C = 2-2*FX;
          float R = (sqrtf(2)*sqrtf(-powf(FX-1, 3))+Res*(FX-1))/(Res*(FX-1));

          State->Freq = Freq;
          State->Res = Res;
          State->FX = FX;
          State->C = C;
          State->R = R;
     }
     State->Speed += (In - State->Pos) * State->C;
     State->Pos += State->Speed;
     State->Speed *= State->R;
     return State->Pos;
}
