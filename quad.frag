#version 410 core

uniform samplerBuffer AudioL;

in vec2 vUV;

out vec4 fragColor;

void main() {
    float R, G, B;
    R=G=B=0;
    // B = vUV.y;
    // R = abs(uAudioL[int(vUV.y * 1024)])*5;
    R = texelFetch(AudioL, int(vUV.y * 1024)).r*5;
    if (R < 0) B = -R;
    // G = (int(vUV.y * 1024) > 128) ? 0 : 1;
    fragColor = vec4(R, G, B, 1);
}
