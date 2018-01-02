#version 410 core

uniform float uTime;
uniform sampler2D uTexture;

in vec2 vUV;

out vec4 fragColor;

#define PI 3.14151
#define TAU (2.0*PI)

void main() {
    vec2 FBUV = vUV;
    vec4 Color = texture(uTexture, FBUV) * 0.9;
    Color.g *= 0.8;
    float Freq = TAU*30;
    float YOffset = (sin(uTime)*0.5+0.5) * (Freq-PI);
    float Pulse = sin(clamp(vUV.y*Freq-YOffset, 0, PI));
    Pulse *= sin(vUV.x*Freq-uTime*50)*0.5+0.5;

    Color += vec4(Pulse, Pulse, Pulse, 1);
    fragColor = Color;
}


