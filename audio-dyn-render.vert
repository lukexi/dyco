#version 410 core

uniform vec2 Scale;
uniform vec2 Translate;

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

void main() {
    gl_Position = vec4(aPosition*Scale+Translate, 0.0, 1.0);
    vUV = aUV;
}
