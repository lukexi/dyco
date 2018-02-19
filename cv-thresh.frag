#version 410 core

uniform sampler2D uTexture;

in vec2 vUV;

out vec4 fragColor;

void main() {
    ivec2 Location = ivec2(gl_FragCoord.xy);

    vec4 Pixel = texelFetch(uTexture, Location, 0);
    vec4 Color = length(Pixel.rgb) > 0.8 ? Pixel : vec4(0);

    fragColor = Color;
    // fragColor = vec4(1,0,0,1);
}

