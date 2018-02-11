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
    scope Scopes[3];
} r_state;

void Cleanup() {
    // glDeleteProgram(State->Shader);
    // glDeleteVertexArrays(1, &QuadVAO);

    // for (int I = 0; I < 3; I++) {
    //     glDeleteBuffers(1, &Scopes[I].TexBuf);
    //     glDeleteTextures(1, &Scopes[I].Tex);
    // }
}

void LoadShader(r_state* State) {
    time_t NewShaderModTime = GetFileModTime("audio-render.frag");
    if (NewShaderModTime > State->ShaderModTime) {
        State->ShaderModTime = NewShaderModTime;
        glDeleteProgram(State->Shader);
        State->Shader = CreateVertFragProgramFromPath("quad.vert", "audio-render.frag");
        glUseProgram(State->Shader);

        glUniform1i(glGetUniformLocation(State->Shader, "AudioRed"), 0);
        glUniform1i(glGetUniformLocation(State->Shader, "AudioGrn"), 1);
        glUniform1i(glGetUniformLocation(State->Shader, "AudioBlu"), 2);

        glUniform1i(glGetUniformLocation(State->Shader, "BufferSize"), BUFFER_SIZE);
    }
}



r_state* TickRender(SDL_Window* Window, audio_state* AudioState, r_state* State) {
    if (State == NULL) {
        State = calloc(1, sizeof(r_state));
        State->QuadVAO = CreateQuad(FullscreenQuadVertices);

        LoadShader(State);

        scope* Scopes = State->Scopes;
        for (int I = 0; I < 3; I++) {
            InitScope(&Scopes[I]);
        }
    }
    LoadShader(State);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    TickOscilloscope(&AudioState->AudioTapRed, &State->Scopes[0], GL_TEXTURE0 + 0);
    TickOscilloscope(&AudioState->AudioTapGrn, &State->Scopes[1], GL_TEXTURE0 + 1);
    TickOscilloscope(&AudioState->AudioTapBlu, &State->Scopes[2], GL_TEXTURE0 + 2);

    glBindVertexArray(State->QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);

    return State;
}



