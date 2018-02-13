#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "audio-dyn-interface.h"
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
        State->Shader = CreateVertFragProgramFromPath("audio-dyn-render.vert", "audio-dyn-render.frag");
        glUseProgram(State->Shader);

    }
}

void DrawUnit(r_state* State, audio_unit* Unit, float X, float Y) {
    if (!Unit) return;
    if (Unit->Scope.Tex == 0) {
        InitScope(&Unit->Scope);
    }
    float Scale = 0.15;
    float YOffset = 0.35;
    glUniform2f(glGetUniformLocation(State->Shader, "Scale"), Scale,Scale);
    glUniform2f(glGetUniformLocation(State->Shader, "Translate"), X,Y);
    glUniform1i(glGetUniformLocation(State->Shader, "BufferSize"), BUFFER_SIZE);

    TickOscilloscope(
        &Unit->ScopeBuffer,
        &Unit->Scope,
        GL_TEXTURE0+0);

    glBindVertexArray(State->QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    for (int I = 0; I < ARRAY_LEN(Unit->Inputs); I++) {
        DrawUnit(State, Unit->Inputs[I].Unit, I*(YOffset), Y + YOffset);
    }
}

void TickRender(SDL_Window* Window, audio_state* AudioState) {

    static bool Initialized = false;
    if (!Initialized) {
        Initialized = true;
        State.QuadVAO = CreateQuad(FullscreenQuadVertices);
    }
    LoadShader(&State);

    glClearColor(0.8, 0.6, 0.9, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    DrawUnit(&State, AudioState->OutputUnit, 0, -0.75);

    SwapWindowQ(Window);
}



