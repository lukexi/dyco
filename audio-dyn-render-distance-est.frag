#version 410 core

uniform int BufferSize;
uniform samplerBuffer AudioBuffer;
in vec2 vUV;
out vec4 fragColor;

float ReadTap(float X) {
    float ReadPoint = X * BufferSize;
    float Offset = fract(ReadPoint);
    int ReadIndex0 = int(X * BufferSize);
    int ReadIndex1 = ReadIndex0 + 1;

    return (texelFetch(AudioBuffer, ReadIndex0).r * (1-Offset)
         + texelFetch(AudioBuffer, ReadIndex1).r * Offset)
        *0.5+0.5;
    // return sin(X*3.14);
}

float de(const in vec2 p)
{
    float v = ReadTap(p.x)-p.y;
    float h = .5;
    float g = 1.5+ pow(ReadTap(p.x+h) - ReadTap(p.x-h),2.);
    float de = abs(v)/sqrt(g);
    return float(smoothstep( 0., .13, de ));
}

float draw(const in vec2 p, const in float zoom)
{
    float thickness = 1;
    float rz = de(p);
    rz *= (1./thickness)/sqrt(zoom/768.0);
    rz = 1.-clamp(rz, 0., 1.);
    return rz;
}



void main() {
    float R=0, G=0, B=0;
    // float Wave = ReadTap(vUV);

    // Unipolar
    // R += Wave > 0 ? Wave : 0;
    // // G += abs(Wave);
    // B += Wave < 0 ? -Wave : 0;
    G += draw(vUV,1);

    fragColor = vec4(R, G, B, 1);
}
