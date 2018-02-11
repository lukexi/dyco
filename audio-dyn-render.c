#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "audio-interface.h"
#include "shader.h"
#include "quad.h"
#include "dynamic.h"
#include "utils.h"
#include "audio-oscilloscope.c"

typedef struct {
    GLuint QuadVAO;
    GLuint Shader;
    time_t ShaderModTime;
} r_state;

static r_state State;

void Cleanup() {
    glDeleteProgram(State.Shader);
    glDeleteVertexArrays(1, &State.QuadVAO);
}

void LoadShader(r_state* State) {
    time_t NewShaderModTime = GetFileModTime("audio-dyn-render.frag");
    if (NewShaderModTime > State->ShaderModTime) {
        State->ShaderModTime = NewShaderModTime;
        glDeleteProgram(State->Shader);
        State->Shader = CreateVertFragProgramFromPath("quad.vert", "audio-dyn-render.frag");
        glUseProgram(State->Shader);
    }
}


void TickRender(SDL_Window* Window, audio_state* AudioState) {

    static bool Initialized = false;
    if (!Initialized) {
        Initialized = true;
        State.QuadVAO = CreateQuad(FullscreenQuadVertices);
    }
    LoadShader(&State);

    glClearColor(0, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(State.QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);

}



