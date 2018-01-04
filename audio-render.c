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
    float Level; // Value to trigger at (on rising slope)
    int   NextScanIndex;
    float LastVal;

    Level         = 0;
    NextScanIndex = 0;
    LastVal       = 1;

    for (int I = 0; I < Length; I++) {
        float Val = Samples[I];
        if (LastVal < Level && Val >= Level)  {
            return NextScanIndex;
        }
        LastVal = Val;
        NextScanIndex++;
    }
    return 0;
}



bool Initialized = false;

GLuint QuadVAO;
GLuint Shader;
time_t ShaderModTime;

#define BUFFER_SIZE 512

int AudioBufIndexes[3];
GLuint AudioBufs[3];
GLuint AudioTexs[3];

float TempBuffers[3][BUFFER_SIZE*2];

void Cleanup() {
    glDeleteProgram(Shader);
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(3, AudioBufs);
    glDeleteTextures(3, AudioTexs);
}

void LoadShader() {
    time_t NewShaderModTime = GetFileModTime("audio-render.frag");
    if (NewShaderModTime > ShaderModTime) {
        ShaderModTime = NewShaderModTime;
        glDeleteProgram(Shader);
        Shader = CreateVertFragProgramFromPath("quad.vert", "audio-render.frag");
        glUseProgram(Shader);

        glUniform1i(glGetUniformLocation(Shader, "AudioRed"), 0);
        glUniform1i(glGetUniformLocation(Shader, "AudioGrn"), 1);
        glUniform1i(glGetUniformLocation(Shader, "AudioBlu"), 2);


        glUniform1i(glGetUniformLocation(Shader, "BufferSize"), BUFFER_SIZE);
    }
}

void Initialize() {

    QuadVAO = CreateQuad(FullscreenQuadVertices);

    LoadShader();


    glGenBuffers(3, AudioBufs);
    glGenTextures(3, AudioTexs);

    for (int I = 0; I < 3; I++) {
        glBindTexture(GL_TEXTURE_BUFFER, AudioTexs[I]);
        glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[I]);
        glBufferData(GL_TEXTURE_BUFFER, BUFFER_SIZE * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, AudioBufs[I]);
    }

    Initialized = true;
}

void TickAudioTap(ringbuffer* AudioTap, int ID) {
    float* TempBuffer = TempBuffers[ID];
    while (GetRingBufferReadAvailable(AudioTap) >= 1) {
        audio_block Block;
        ReadRingBuffer(AudioTap, &Block, 1);

        bool ResetIndex = (AudioBufIndexes[ID] + Block.Length) > (BUFFER_SIZE*2);
        if (ResetIndex) {
            int TriggerPoint = FindOscilloscopeTrigger(TempBuffer, BUFFER_SIZE*2);

            glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[ID]);
            glBufferSubData(GL_TEXTURE_BUFFER,
                0,
                BUFFER_SIZE*sizeof(float),
                TempBuffer+TriggerPoint);

            AudioBufIndexes[ID] = 0;

            // Should move the data after what we uploaded to the beginning of the TempBuffer
            // and set AudioBufIndexes to its length.
            // float Tmp[BUFFER_SIZE];
            // size_t AmountToCopy = BUFFER_SIZE*2 - (TriggerPoint+BUFFER_SIZE);
            // memcpy(Tmp, TempBuffer+TriggerPoint+BUFFER_SIZE, AmountToCopy*sizeof(float));
            // memcpy(TempBuffer, Tmp, AmountToCopy*sizeof(float));
            // AudioBufIndexes[ID] = AmountToCopy;
        }

        for (int I = 0; I < Block.Length; I++) {
            TempBuffer[AudioBufIndexes[ID]] = Block.Samples[I];
            AudioBufIndexes[ID]++;
        }

        free(Block.Samples);
        free(Block.Freqs);
    }

    glActiveTexture(GL_TEXTURE0 + ID);
    glBindTexture(GL_TEXTURE_BUFFER, AudioTexs[ID]);
}

void TickRender(SDL_Window* Window, audio_state* AudioState) {
    if (!Initialized) Initialize();
    LoadShader();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    TickAudioTap(&AudioState->AudioTapRed, 0);
    TickAudioTap(&AudioState->AudioTapGrn, 1);
    TickAudioTap(&AudioState->AudioTapBlu, 2);

    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);
}



