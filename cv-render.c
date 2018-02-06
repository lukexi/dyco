#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "gl.h"
#include "shader.h"
#include "quad.h"
#include "dynamic.h"
#include "utils.h"
#include "texture.h"
#include "framebuffer.h"

GLuint ImageTexID;

char* QuadVertSource;
GLuint QuadVAO;

GLuint QuadProgram; time_t QuadProgramMod;
GLuint SobelProgram;  time_t SobelProgramMod;  framebuffer SobelFB;
GLuint ThreshProgram; time_t ThreshProgramMod; framebuffer ThreshFB;
GLuint PeaksProgram;  time_t PeaksProgramMod;  framebuffer PeaksFB;

void LoadShader(GLuint* Program, time_t* ProgramMod, char* QuadVertSource, char* FragFile) {
    time_t NewShaderModTime = GetFileModTime(FragFile);
    if (NewShaderModTime <= *ProgramMod) return;
    *ProgramMod = NewShaderModTime;
    *Program    = CreateVertFragProgram(QuadVertSource, ReadFile(FragFile));
}

void LoadShaders() {
    LoadShader(&QuadProgram,   &QuadProgramMod,   QuadVertSource, "cv-output.frag");
    LoadShader(&SobelProgram,  &SobelProgramMod,  QuadVertSource, "cv-sobel.frag");
    LoadShader(&ThreshProgram, &ThreshProgramMod, QuadVertSource, "cv-thresh.frag");
    LoadShader(&PeaksProgram,  &PeaksProgramMod,  QuadVertSource, "cv-peaks.frag");
}

void Initialize() {
    static bool Initialized = false;
    if (Initialized) return;
    Initialized = true;

    int ImageWidth, ImageHeight, ImageChannels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *ImageData = stbi_load("IMG_3169.jpg", &ImageWidth, &ImageHeight, &ImageChannels, 0);

     // Create a fullscreen quad
    QuadVertSource = ReadFile("quad.vert");
    QuadVAO = CreateQuad(FullscreenQuadVertices);

    // Load the source image
    ImageTexID = CreateTexture(ImageWidth, ImageHeight, ImageChannels);
    UpdateTexture(ImageTexID, ImageWidth, ImageHeight, GL_RGB, ImageData);

    InitFramebuffer(ImageWidth, ImageHeight, &SobelFB);
    InitFramebuffer(ImageWidth, ImageHeight, &ThreshFB);
    InitFramebuffer(ImageWidth, ImageHeight, &PeaksFB);
}

void DoPass(GLuint InputTexID, GLuint Program, GLuint OutputFBID) {
    glUseProgram(Program);
    glBindTexture(GL_TEXTURE_2D, InputTexID);
    glBindFramebuffer(GL_FRAMEBUFFER, OutputFBID);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void TickRender(SDL_Window* Window) {
    Initialize();
    LoadShaders();

    GLuint InputTexID = ImageTexID;

    DoPass(InputTexID, SobelProgram,  SobelFB.Framebuffer);  InputTexID = SobelFB.FramebufferTex;
    DoPass(InputTexID, ThreshProgram, ThreshFB.Framebuffer); InputTexID = ThreshFB.FramebufferTex;
    // DoPass(InputTexID, PeaksProgram,  PeaksFB.Framebuffer);  InputTexID = PeaksFB.FramebufferTex;

    DoPass(InputTexID, QuadProgram, 0);

    SwapWindowQ(Window);
}
