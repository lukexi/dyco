#version 410 core

uniform samplerBuffer AudioL;
uniform samplerBuffer AudioR;

uniform int IndexL;
uniform int IndexR;

in vec2 vUV;

out vec4 fragColor;

void main() {
    float R, G, B;
    R=G=B=0;
    R = texelFetch(AudioL, (int(vUV.y * 1024) + IndexL) % 1024 ).r*5;
    if (R < 0) B = -R;
    G = texelFetch(AudioR, (int(vUV.y * 1024) + IndexR) % 1024 ).r*5;
    if (G < 0) B = B + -G;
    fragColor = vec4(R, G, B, 1);
}
