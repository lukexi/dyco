#version 410 core

uniform sampler2D uTexture;

out vec4 fragColor;

void main() {


    vec4 Color = texelFetch(uTexture,
        ivec2(gl_FragCoord.xy),
        0);
    vec4 Color2;
    Color2.rgba = Color.rbga;
    fragColor = Color2;
}


