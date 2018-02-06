#version 410 core

uniform sampler2D uTexture;

in vec2 vUV;

out vec4 fragColor;

vec4 FindPeak(ivec2 Location, ivec2 Direction) {
    vec4 Color = vec4(0);
    float[3] Mags;
    for (int i = 0; i < 3; i++) {

        ivec2 Offset = (i-1) * Direction; // Look to the left and right of this pixel (or up and down)

        vec4 Pixel = texelFetch(uTexture, Location+Offset, 0);
        float Magnitude = length(Pixel); // FIXME probably just do a simple sum rather than length
        Mags[i] = Magnitude;
    }
    if (Mags[0] < Mags[1] && Mags[1] > Mags[2]) {
        Color = vec4(1);
    }
    return Color;
}

void main() {
    ivec2 Location = ivec2(gl_FragCoord.xy);

    fragColor = FindPeak(Location, ivec2(1,0)) + FindPeak(Location, ivec2(0,1));
}

