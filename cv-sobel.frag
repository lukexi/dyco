#version 410 core
uniform sampler2D uTexture;
in vec2 vUV;
out vec4 fragColor;

mat3 Kernel = mat3(
   1, 2, 1,
   2, 4, 2,
   1, 2, 1);

mat3 SobelX = mat3(
   -1,  0,  1,
   -2,  0,  2,
   -1,  0,  1);
mat3 SobelY = mat3(
   -1, -2, -1,
    0,  0,  0,
    1,  2,  1);

vec4 Convolve3x3(ivec2 Pixel, mat3 Kernel) {
    vec4 Color = vec4(0);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {

            vec4 Mult = vec4(Kernel[i][j]);

            ivec2 Offset = ivec2(i-1,j-1);

            Color += texelFetch(uTexture, Pixel+Offset, 0) * Mult;
        }
    }
    Color = abs(Color);
    Color /= vec4(2);
    return Color;
}

void main() {
    ivec2 Pixel = ivec2(gl_FragCoord.xy);
    // Pixel.y = textureSize(uTexture, 0).y - Pixel.y;

    fragColor = Convolve3x3(Pixel, SobelX) + Convolve3x3(Pixel, SobelY);
    // fragColor = texelFetch(uTexture, Pixel, 0);
}

