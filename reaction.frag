#version 410 core

uniform sampler2D uTexture;

out vec4 fragColor;

vec2 C(int X, int Y) {
    return texelFetch(uTexture,
        ivec2(gl_FragCoord.xy) + ivec2(X,Y),
        0).rg;
}

float hash(vec2 p)  // replace this by something better
{
    p  = 50.0*fract( p*0.3183099 + vec2(0.71,0.113));
    return -1.0+2.0*fract( p.x*p.y*(p.x+p.y) );
}

float noise(vec2 p)
{
    vec2 i = floor( p );
    vec2 f = fract( p );

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( hash( i + vec2(0.0,0.0) ),
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ),
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}



void main() {
    vec2 C0 = C(0,0);
    vec2 LapC = (C(1,0) + C(-1,0) + C(0,1) + C(0,-1) - 4*C0);

    float Feed = 0.099; // Green
    float Kill = 0.05;
    float DiffR = 0.3097; // Diffusion1
    float DiffG = 0.405;  // Diffusion2
    vec2 C1 = vec2(
        DiffR * LapC.r - C0.r*C0.g*C0.g + Feed*(1.0 - C0.r),
        DiffG * LapC.g + C0.r*C0.g*C0.g - (Feed+Kill)*C0.g);

    float Delta = 0.016;
    Delta = 0.7;
    vec2 Result = C0 + Delta*C1;
    // Result.g = sin(gl_FragCoord.x*0.05) + sin(gl_FragCoord.y*0.05);
    // Result.r = noise(gl_FragCoord.xy/100);
    // Result.g = noise(gl_FragCoord.xy/100);

    fragColor = vec4(Result,0,1);
}


