#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "gl.h"
#include "audio-interface.h"
#include "shader.h"
#include "quad.h"
#include "dynamic.h"
#include "utils.h"

// Returns trigger index or -1 if not found yet.
int FindOscilloscopeTrigger(float* Samples, int Length) {
    const float Level = 0; // Value to trigger at (on rising slope)
    for (int I = 0; I < Length; I++) {
        const float V1 = Samples[I];
        const float V0 = I > 0 ? Samples[I-1] : 1;
        if (V0 < Level && V1 >= Level) return I;
    }
    return 0;
}


#define BUFFER_SIZE 512

#define BUFFER_SIZE_2X (BUFFER_SIZE*2)

typedef struct {
    float  LocalBuf[BUFFER_SIZE_2X];
    int    WriteIndex;
    GLuint TexBuf;
    GLuint Tex;
    GLenum TexUnit;
} scope;







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


void TickOscilloscope(ringbuffer* AudioTap, scope* Scope) {

    while (GetRingBufferReadAvailable(AudioTap) >= 1) {
        audio_block Block;
        ReadRingBuffer(AudioTap, &Block, 1);

        bool ResetIndex = (Scope->WriteIndex + Block.Length) > (BUFFER_SIZE_2X);
        if (ResetIndex) {
            int TriggerPoint = FindOscilloscopeTrigger(Scope->LocalBuf, BUFFER_SIZE_2X);
            if (Scope->WriteIndex - TriggerPoint > BUFFER_SIZE) {
                glBindBuffer(GL_TEXTURE_BUFFER, Scope->TexBuf);
                glBufferSubData(GL_TEXTURE_BUFFER,
                    0,
                    BUFFER_SIZE*sizeof(float),
                    &Scope->LocalBuf[TriggerPoint]);

                // Move the data after what we uploaded to the beginning of the TempBuffer
                // and set AudioBufIndexes to its length.
                size_t AmountToCopy = Scope->WriteIndex - (TriggerPoint+BUFFER_SIZE);
                memmove(&Scope->LocalBuf[0],
                        &Scope->LocalBuf[TriggerPoint+BUFFER_SIZE],
                        AmountToCopy*sizeof(float));

                Scope->WriteIndex = AmountToCopy;
            } else {
                Scope->WriteIndex = 0;
            }
        }

        for (int I = 0; I < Block.Length; I++) {
            Scope->LocalBuf[Scope->WriteIndex++] = Block.Samples[I];
        }

        free(Block.Samples);
        free(Block.Freqs);
    }

    glActiveTexture(Scope->TexUnit);
    glBindTexture(GL_TEXTURE_BUFFER, Scope->Tex);
}

r_state* TickRender(SDL_Window* Window, audio_state* AudioState, r_state* State) {
    if (State == NULL) {
        State = calloc(1, sizeof(r_state));
        State->QuadVAO = CreateQuad(FullscreenQuadVertices);

        LoadShader(State);

        scope* Scopes = State->Scopes;
        for (int I = 0; I < 3; I++) {

            Scopes[I].TexUnit = GL_TEXTURE0 + I;

            glGenBuffers(1, &Scopes[I].TexBuf);
            glGenTextures(1, &Scopes[I].Tex);

            glBindTexture(GL_TEXTURE_BUFFER, Scopes[I].Tex);
            glBindBuffer(GL_TEXTURE_BUFFER, Scopes[I].TexBuf);
            glBufferData(GL_TEXTURE_BUFFER,
                BUFFER_SIZE * sizeof(float),
                NULL, GL_DYNAMIC_DRAW);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, Scopes[I].TexBuf);
        }
    }
    LoadShader(State);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    TickOscilloscope(&AudioState->AudioTapRed, &State->Scopes[0]);
    TickOscilloscope(&AudioState->AudioTapGrn, &State->Scopes[1]);
    TickOscilloscope(&AudioState->AudioTapBlu, &State->Scopes[2]);

    glBindVertexArray(State->QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);

    return State;
}



