#ifndef AUDIO_OSCILLOSCOPE_H
#define AUDIO_OSCILLOSCOPE_H

#include "gl.h"
#include "ringbuffer.h"

#define BUFFER_SIZE 512

#define BUFFER_SIZE_2X (BUFFER_SIZE*2)

typedef struct {
    float  LocalBuf[BUFFER_SIZE_2X];
    int    WriteIndex;
    GLuint TexBuf;
    GLuint Tex;
} scope;

void InitScope(scope* Scope);
void TickOscilloscope(ringbuffer* AudioTap, scope* Scope, GLenum TexUnit);
void FreeScope(scope* Scope);

#endif // AUDIO_OSCILLOSCOPE_H
