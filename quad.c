#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

GLuint CreateQuad(const float* QuadVertices) {

    GLuint VAO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);


    /*

    1__3
    | /|
    |/_|
    2  4

    */
    const int NumVertComponents = 2;

    GLuint VertBuffer;
    glGenBuffers(1, &VertBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices)*8, QuadVertices, GL_STATIC_DRAW);

    const GLuint PositionAttrIndex = 0; // layout(location = 0) in vert shader
    glVertexAttribPointer(
        PositionAttrIndex,  // Attribute Index
        NumVertComponents,  // Attribute Size
        GL_FLOAT,           // Attribute Type
        GL_FALSE,           // Normalize values?
        0,                  // Stride
        0                   // Pointer to first item
        );
    glEnableVertexAttribArray(PositionAttrIndex);


    // UVs
    static float QuadUVs[8] = {
        0, 1, // Left Top
        0, 0, // Left Bottom
        1, 1, // Right Top
        1, 0  // Right Bottom
    };

    const int NumUVComponents = 2;

    GLuint UVsBuffer;
    glGenBuffers(1, &UVsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, UVsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadUVs), QuadUVs, GL_STATIC_DRAW);

    const GLuint UVsAttrIndex = 1; // layout(location = 1) in vert shader
    glVertexAttribPointer(
        UVsAttrIndex,       // Attribute Index
        NumUVComponents,    // Attribute Size
        GL_FLOAT,           // Attribute Type
        GL_FALSE,           // Normalize values?
        0,                  // Stride
        0                   // Pointer to first item
        );
    glEnableVertexAttribArray(UVsAttrIndex);

    return VAO;
}
