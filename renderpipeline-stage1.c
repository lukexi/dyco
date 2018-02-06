#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "shader.h"
#include "quad.h"
#include "dynamic.h"
#include "framebuffer.h"
#include "renderpipeline-interface.h"

bool Initialized = false;

GLuint Shader;
time_t ShaderModTime;
GLuint uTime;
GLuint uTexture;

void Cleanup() {
    glDeleteProgram(Shader);
}

void LoadShader() {
    time_t NewShaderModTime = GetFileModTime("renderpipeline-stage1.frag");
    if (NewShaderModTime > ShaderModTime) {
        ShaderModTime = NewShaderModTime;
        glDeleteProgram(Shader);
        Shader = CreateVertFragProgramFromPath("quad.vert", "renderpipeline-stage1.frag");
        uTime = glGetUniformLocation(Shader, "uTime");
        uTexture = glGetUniformLocation(Shader, "uTexture");
        glUniform1i(uTexture, 0);
    }
    glUseProgram(Shader);
}

void Initialize() {
    LoadShader();

    Initialized = true;
}

void TickStage(SDL_Window* Window, pipeline_state* PipelineState) {
    if (!Initialized) Initialize();
    LoadShader();

    glBindFramebuffer(GL_FRAMEBUFFER,
        PipelineState->NewFramebuffer.Framebuffer);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,
        PipelineState->OldFramebuffer.FramebufferTex);

    glUniform1f(uTime, SDL_GetTicks()/(float)1000.0);

    glBindVertexArray(PipelineState->QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);
}
