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
GLuint uAudioIndexL;
GLuint uAudioIndexR;

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
        ShaderModTime = NewShaderModTime;
        glDeleteShader(Program);
        Program = CreateVertFragProgramFromPath("quad.vert", "quad.frag");
        glUseProgram(Program);

        GLuint uAudioTexL = glGetUniformLocation(Program, "AudioL");
        GLuint uAudioTexR = glGetUniformLocation(Program, "AudioR");
        glUniform1i(uAudioTexL, 0);
        glUniform1i(uAudioTexR, 1);
        uAudioIndexL = glGetUniformLocation(Program, "IndexL");
        uAudioIndexR = glGetUniformLocation(Program, "IndexR");
    }
}

void Initialize() {

    QuadVAO = CreateQuad(FullscreenQuadVertices);

    LoadShader();


    glGenBuffers(2, AudioBufs);
    glGenTextures(2, AudioTexs);

    glBindTexture(GL_TEXTURE_BUFFER, AudioTexs[0]);
    glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[0]);
    glBufferData(GL_TEXTURE_BUFFER, 1024 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, AudioBufs[0]);

    glBindTexture(GL_TEXTURE_BUFFER, AudioTexs[1]);
    glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[1]);
    glBufferData(GL_TEXTURE_BUFFER, 1024 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, AudioBufs[1]);



    Initialized = true;
}

void TickRender(SDL_Window* Window, audio_state* AudioState) {
    if (!Initialized) Initialize();
    LoadShader();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);


    while (GetRingBufferReadAvailable(&AudioState->AudioTapL) >= 1) {
        audio_block Block;
        ReadRingBuffer(&AudioState->AudioTapL, &Block, 1);
        bool ResetIndex = IndexL + Block.Length > 1024;
        if (ResetIndex) IndexL = 0;

        glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[0]);
        glBufferSubData(GL_TEXTURE_BUFFER, IndexL*sizeof(float), Block.Length*sizeof(float), Block.Samples);

        static int ScanIndexL = 0;
        static bool FoundZeroL = false;
        static float LastValL = 1;
        if (ResetIndex) {
            FoundZeroL = false;
            ScanIndexL = 0;
            LastValL = 1;
        }

        if (!FoundZeroL) {
            const float Trigger = 0;
            for (int I = 0; I < Block.Length; I++) {
                float Val = Block.Samples[I];
                if (LastValL < Trigger && Val >= Trigger)  {
                    FoundZeroL = true;
                    break;
                }
                LastValL = Val;
                ScanIndexL++;
            }
        }

        IndexL += Block.Length;
        glUniform1i(uAudioIndexL, ScanIndexL);
        free(Block.Samples);
    }

    while (GetRingBufferReadAvailable(&AudioState->AudioTapR) >= 1) {
        audio_block Block;
        ReadRingBuffer(&AudioState->AudioTapR, &Block, 1);
        bool ResetIndex = IndexR + Block.Length > 1024;
        if (ResetIndex) IndexR = 0;

        glBindBuffer(GL_TEXTURE_BUFFER, AudioBufs[1]);
        glBufferSubData(GL_TEXTURE_BUFFER, IndexR*sizeof(float), Block.Length*sizeof(float), Block.Samples);

        static int ScanIndexR = 0;
        static bool FoundZeroR = false;
        static float LastValR = 1;
        if (ResetIndex) {
            FoundZeroR = false;
            ScanIndexR = 0;
            LastValR = 1;
        }

        if (!FoundZeroR) {
            const float Trigger = 0;
            for (int I = 0; I < Block.Length; I++) {
                float Val = Block.Samples[I];
                if (LastValR < Trigger && Val >= Trigger)  {
                    FoundZeroR = true;
                    break;
                }
                LastValR = Val;
                ScanIndexR++;
            }
        }

        IndexR += Block.Length;
        glUniform1i(uAudioIndexR, ScanIndexR);
        free(Block.Samples);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, AudioTexs[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, AudioTexs[1]);

    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapWindowQ(Window);
}
