#version 410 core

uniform int BufferSize;
uniform samplerBuffer AudioBuffer;
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
    float Wave = ReadTap(AudioBuffer);

    // Unipolar
    R += abs(Wave);
    G += abs(Wave);
    B += abs(Wave);
    R += Plot(vUV, Wave*0.5+0.5, 0.01);
    G += Plot(vUV, Wave*0.5+0.5, 0.01);
    B += Plot(vUV, Wave*0.5+0.5, 0.01);
    fragColor = vec4(R, G, B, 1);
}
