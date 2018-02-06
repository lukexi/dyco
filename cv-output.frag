#version 410 core

uniform sampler2D uTexture;

in vec2 vUV;

out vec4 fragColor;

void main() {
    fragColor = texture(uTexture, vUV);
}

