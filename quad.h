#ifndef QUAD_H
#define QUAD_H

static float FullscreenQuadVertices[8] = {
    -1, 1, // Left Top
    -1, -1,  // Left Bottom
    1, 1,  // Right Top
    1, -1    // Right Bottom
};

GLuint CreateQuad(const float* QuadVertices);

#endif // QUAD_H
