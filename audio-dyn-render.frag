#version 410 core

uniform int BufferSize;
uniform samplerBuffer AudioBuffer;
in vec2 vUV;
out vec4 fragColor;

float ReadTap(float X) {
    X = clamp(X, 0, 1);
    float ReadIndex = X * (BufferSize-1);
    float Offset = fract(ReadIndex);
    int ReadIndex0 = int(ReadIndex);
    int ReadIndex1 = min(ReadIndex0 + 1, BufferSize-1);

    return (texelFetch(AudioBuffer, ReadIndex0).r * (1-Offset)
         + texelFetch(AudioBuffer, ReadIndex1).r * Offset)
        *0.5+0.5;
    // return sin(X*3.14);
}

float rand(vec2 co){
    // implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Put your user defined function here...
float function(float x) {
    return ReadTap(x);
}

float Jitter=1; //slider[0,0.5,2]
float Detail=10; //slider[0,5,20]
int Samples=2; //slider[0,3,100]
// float AxisDetail=10; //slider[1,1,10]
vec2 aaScale=vec2(0.001);
vec3 getColor2D(vec2 pos) {
    vec2 step = Detail*vec2(aaScale.x,aaScale.y)/Samples;
    float samples = float(Samples);

    int count = 0;
    int mySamples = 0;

    for (float i = 0.0; i < samples; i++) {
        for (float  j = 0.0;j < samples; j++) {
            if (i*i+j*j>samples*samples) continue;
            mySamples++;
            float ii = i + Jitter*rand(vec2(pos.x+ i*step.x,pos.y+ j*step.y));
            float jj = j + Jitter*rand(vec2(pos.y + i*step.x,pos.x+ j*step.y));
            float f = function(pos.x+ ii*step.x)-(pos.y+ jj*step.y);
            count += (f>0.) ? 1 : -1;
        }
    }
    vec3 color = vec3(1.0);
    if (abs(count)!=mySamples) color =
        vec3(abs(float(count))/float(mySamples));
    return 1-color;
}


void main() {
    float R=0, G=0, B=0;
    // float Wave = ReadTap(vUV);

    // Unipolar
    // R += Wave > 0 ? Wave : 0;
    // // G += abs(Wave);
    // B += Wave < 0 ? -Wave : 0;
    // G += draw(vUV,1);

    fragColor = vec4(getColor2D(vUV), 1);
}
