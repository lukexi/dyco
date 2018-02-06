#version 410 core

uniform int BufferSize;
uniform samplerBuffer AudioRed;
uniform samplerBuffer AudioGrn;
uniform samplerBuffer AudioBlu;
in vec2 vUV;
out vec4 fragColor;

float Plot(vec2 UV, float Y, float Thickness) {
    return smoothstep(Y-Thickness, Y, UV.y) -
           smoothstep(Y, Y+Thickness, UV.y);
}

float ReadTap(samplerBuffer Samples) {
    float ReadPoint = vUV.x * BufferSize;
    float Offset = fract(ReadPoint);
    int ReadIndex0 = int(vUV.x * BufferSize);
    int ReadIndex1 = ReadIndex0 + 1;

    return texelFetch(Samples, ReadIndex0).r * (1-Offset)
         + texelFetch(Samples, ReadIndex1).r * Offset;
}

void main() {
    float R=0, G=0, B=0;
    float RedWave = ReadTap(AudioRed);
    float GrnWave = ReadTap(AudioGrn);
    float BluWave = ReadTap(AudioBlu);
    // Pos/Neg (arbitrary colors for negative)
    // R+=RedWave>0?RedWave:0;
    // G+=GrnWave>0?GrnWave:0;
    // B+=BluWave>0?BluWave:0;
    // R+=BluWave<0?-BluWave:0;
    // G+=GrnWave<0?-GrnWave:0;
    // B+=RedWave<0?-RedWave:0;

    // Unipolar
    R += abs(RedWave);
    G += abs(GrnWave);
    B += abs(BluWave);
    R += Plot(vUV, RedWave*0.5+0.5, 0.01);
    G += Plot(vUV, GrnWave*0.5+0.5, 0.01);
    B += Plot(vUV, BluWave*0.5+0.5, 0.01);
    fragColor = vec4(R, G, B, 1);
}
