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
} filter_rlop;

float TickFilter(
     filter_rlop* Filter, float Freq, float Res,
     float SampleRate, float In)
{
     if (Freq != Filter->Freq || Res != Filter->Res) {
          float FX = cos(2*M_PI*Freq / SampleRate);
          float C = 2-2*FX;
          float R = (sqrtf(2)*sqrtf(-powf(FX-1, 3))+Res*(FX-1))/(Res*(FX-1));

          Filter->Freq = Freq;
          Filter->Res = Res;
          Filter->FX = FX;
          Filter->C = C;
          Filter->R = R;
     }
     Filter->Speed += (In - Filter->Pos) * Filter->C;
     Filter->Pos += Filter->Speed;
     Filter->Speed *= Filter->R;
     return Filter->Pos;
}
