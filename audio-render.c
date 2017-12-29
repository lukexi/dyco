#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "audio-interface.h"
#include "shader.h"
#include "quad.h"
#include "dynamic.h"

bool Initialized = false;

GLuint QuadVAO;
GLuint Program;

float AudioL[1024]; int IndexL = 0;
float AudioR[1024]; int IndexR = 0;

GLuint AudioBufs[2];
GLuint AudioTexs[2];

time_t ShaderModTime;

void Cleanup() {
    glDeleteShader(Program);
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(2, AudioBufs);
    glDeleteTextures(2, AudioTexs);
}

void LoadShader() {
    time_t NewShaderModTime = GetFileModTime("quad.frag");
    if (NewShaderModTime > ShaderModTime) {
        glDeleteShader(Program);
        Program = CreateVertFragProgramFromPath("quad.vert", "quad.frag");
        ShaderModTime = NewShaderModTime;
    }
}

void Initialize() {

    QuadVAO = CreateQuad(FullscreenQuadVertices);

    LoadShader();
    glUseProgram(Program);


    glGenBuffers(2, AudioBufs);
    glGenTextures(2, AudioTexs);

    glBindTexture(GL_TEXTURE_BUFFER, AudioTexs[0]);
    glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[0]);
    glBufferData(GL_TEXTURE_BUFFER, 1024 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, AudioBufs[0]);

    Initialized = true;
}

void TickRender(SDL_Window* Window, audio_state* AudioState) {
    if (!Initialized) Initialize();
    LoadShader();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    while (GetRingBufferReadAvailable(&AudioState->AudioTapL) >= 1) {
        audio_block BlockL;
        ReadRingBuffer(&AudioState->AudioTapL, &BlockL, 1);
        if (IndexL + BlockL.Length > 1024) IndexL = 0;

        glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[0]);
        glBufferSubData(GL_TEXTURE_BUFFER, IndexL*sizeof(float), BlockL.Length*sizeof(float), BlockL.Samples);

        IndexL += BlockL.Length;
        free(BlockL.Samples);
    }

    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);
}
