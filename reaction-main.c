#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "shader.h"
#include "quad.h"
#include "framebuffer.h"
#include "dynamic.h"

GLuint QuadVAO;

GLuint ReactionShader;
time_t ReactionShaderModTime;
GLuint DrawShader;
time_t DrawShaderModTime;

static framebuffer OldFramebuffer;
static framebuffer NewFramebuffer;

void LoadShader(char* ShaderFile, GLuint* Shader, time_t* ModTime) {
    time_t NewModTime = GetFileModTime(ShaderFile);
    if (NewModTime > *ModTime) {
        *ModTime = NewModTime;
        glDeleteProgram(*Shader);
        *Shader = CreateVertFragProgramFromPath("quad.vert", ShaderFile);
    }
}

void Cleanup() {
    glDeleteVertexArrays(1, &QuadVAO);
    DeleteFramebuffer(NewFramebuffer);
    DeleteFramebuffer(OldFramebuffer);
    glDeleteProgram(ReactionShader);
    glDeleteProgram(DrawShader);
}

void Initialize() {
    static bool Initialized = false;
    if (Initialized) return;
    Initialized = true;

    QuadVAO = CreateQuad(FullscreenQuadVertices);

    InitFramebuffer(1024*2, 768*2, &NewFramebuffer);
    InitFramebuffer(1024*2, 768*2, &OldFramebuffer);

    Initialized = true;
}

#define SWAP(TYPE, X,Y) ({ TYPE X0 = X; TYPE Y0 = Y; X = Y0; Y = X0; })

void TickReaction(SDL_Window* Window) {
    Initialize();
    LoadShader("reaction.frag", &ReactionShader, &ReactionShaderModTime);
    LoadShader("reaction-draw.frag", &DrawShader, &DrawShaderModTime);

    GLuint uTexture = glGetUniformLocation(ReactionShader, "uTexture");
    glUniform1i(uTexture, 0);

    SWAP(framebuffer, OldFramebuffer, NewFramebuffer);

    glUseProgram(ReactionShader);

    // Set Destination
    glBindFramebuffer(GL_FRAMEBUFFER,
        NewFramebuffer.Framebuffer);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set Source
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,
        OldFramebuffer.FramebufferTex);

    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);



    // Draw
    glUseProgram(DrawShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,
        OldFramebuffer.FramebufferTex);
    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);
}
