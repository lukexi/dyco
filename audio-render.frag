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
    int ReadIndex = int(vUV.x * BufferSize);
    return texelFetch(Samples, ReadIndex).r
        * 0.5 + 0.5;
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
    R+=RedWave*0.5+0.5;
    G+=GrnWave*0.5+0.5;
    B+=BluWave*0.5+0.5;
    R += Plot(vUV, RedWave * 0.5 + 0.25, 0.01);
    G += Plot(vUV, GrnWave * 0.5 + 0.25, 0.01);
    B += Plot(vUV, BluWave * 0.5 + 0.25, 0.01);
    fragColor = vec4(R, G, B, 1);
}
