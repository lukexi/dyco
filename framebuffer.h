#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include <GL/glew.h>

typedef struct {
    GLuint FramebufferTex;
    GLuint FramebufferRB;
    GLuint Framebuffer;
} framebuffer;

void InitFramebuffer(GLsizei Width, GLsizei Height, framebuffer* Framebuffer);
void DeleteFramebuffer(framebuffer Framebuffer);

void CreateFramebuffer(GLenum Storage,
    GLsizei SizeX, GLsizei SizeY,
    GLuint *TextureIDOut,
    GLuint *FramebufferIDOut,
    GLuint *RenderbufferIDOut);

#endif // FRAMEBUFFER_H
