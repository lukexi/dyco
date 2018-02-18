#include "gl.h"
#include "nanovg.h"
#include "stb_image.h"
#include "framebuffer.h"
#include "quad.h"
#include "shader.h"
#define NANOVG_GL3
#include "nanovg_gl.h"
#include <sys/stat.h>
#include <stdbool.h>

int ImageWidth, ImageHeight, ImageChannels;
unsigned char* ImageData;
unsigned char* PassData = NULL;
int NVGImage;
int NVGImage2;

GLuint QuadVAO;

char* QuadVertSource;
GLuint QuadVAO;

GLuint QuadProgram; time_t QuadProgramMod;
GLuint SobelProgram;  time_t SobelProgramMod;  framebuffer SobelFB;
GLuint ThreshProgram; time_t ThreshProgramMod; framebuffer ThreshFB;

time_t GetFileModTime(char* FileName) {
    struct stat attr = { 0 };
    stat(FileName, &attr);
    return attr.st_mtime;
}

bool LoadShader(GLuint* Program, time_t* ProgramMod, char* QuadVertSource, char* FragFile) {
    time_t NewShaderModTime = GetFileModTime(FragFile);
    if (NewShaderModTime <= *ProgramMod) return false;
    *ProgramMod = NewShaderModTime;
    *Program    = CreateVertFragProgram(QuadVertSource, ReadFile(FragFile));
    return true;
}
bool LoadShaders() {
    bool A = LoadShader(&SobelProgram,  &SobelProgramMod,  QuadVertSource, "cv-sobel.frag");
    bool B = LoadShader(&ThreshProgram,  &ThreshProgramMod,  QuadVertSource, "cv-thresh.frag");
    return A||B;
}

void DoPass(GLuint InputTexID, GLuint Program, GLuint OutputFBID) {
    glUseProgram(Program);
    glBindTexture(GL_TEXTURE_2D, InputTexID);
    glBindFramebuffer(GL_FRAMEBUFFER, OutputFBID);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Setup(NVGcontext* NVG) {
    ImageData = stbi_load("IMG_3169.jpg",
        &ImageWidth, &ImageHeight, &ImageChannels, 4);
    NVGImage = nvgCreateImageRGBA(NVG, ImageWidth, ImageHeight, 0, ImageData);

    QuadVertSource = ReadFile("quad.vert");
    QuadVAO = CreateQuad(FullscreenQuadVertices);

    InitFramebuffer(ImageWidth, ImageHeight, &SobelFB);
    InitFramebuffer(ImageWidth, ImageHeight, &ThreshFB);
}

typedef struct {
    union { float X; float R; };
    union { float Y; float G; };
    union { float Z; float B; };
} vec3;

typedef struct {
    float X;
    float Y;
} vec2;

vec3 Pixel(int X, int Y, unsigned char* Image) {
    unsigned char* Pixel = &Image[((Y*ImageWidth)+X)*4];
    return (vec3){.R=Pixel[0]/255.0, .G=Pixel[1]/255.0, .B=Pixel[2]/255.0};
}

void MarchPixel(int X, int Y, unsigned char* Image) {
    vec3 Color = Pixel(X,Y,Image);


}

void Tick(SDL_Window* Window, NVGcontext* NVG) {
    if (LoadShaders()) {

        GLuint InputTexID = nvglImageHandleGL3(NVG, NVGImage);
        DoPass(InputTexID, SobelProgram,   SobelFB.Framebuffer);
        InputTexID = SobelFB.FramebufferTex;
        DoPass(InputTexID, ThreshProgram,  ThreshFB.Framebuffer);

        free(PassData);
        nvgDeleteImage(NVG,NVGImage2);
        PassData = calloc(1, sizeof(unsigned char) * ImageWidth * ImageHeight * 4);

        glBindTexture(GL_TEXTURE_2D, ThreshFB.FramebufferTex);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, PassData);
        NVGImage2 = nvgCreateImageRGBA(NVG, ImageWidth, ImageHeight, 0, PassData);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the image
    nvgBeginFrame(NVG, 1024,768,2);
    nvgBeginPath(NVG);
    NVGpaint ImagePaint = nvgImagePattern(NVG, 0,0, ImageWidth, ImageHeight, 0, NVGImage2, 1);
    nvgRect(NVG, 0,0,ImageWidth,ImageHeight);
    nvgFillPaint(NVG, ImagePaint);
    nvgFill(NVG);

    // Test pixel extraction
    // double Frac;
    // double Saw = modf((double)SDL_GetTicks()/1000.0/2, &Frac);
    // for (int I = 0; I<10; I++) {
    //     int X = Saw*1024;
    //     int Y = 100*I;
    //     NVGcolor Color = Pixel(X,Y, PassData);
    //     nvgBeginPath(NVG);
    //     nvgRect(NVG, X,Y,100,100);
    //     nvgFillColor(NVG, Color);
    //     nvgFill(NVG);
    // }


    nvgEndFrame(NVG);
    SwapWindowQ(Window);
}

void Cleanup(NVGcontext* NVG) {
    stbi_image_free(ImageData);
    nvgDeleteImage(NVG, NVGImage);
    nvgDeleteImage(NVG, NVGImage2);
    DeleteFramebuffer(SobelFB);
    glDeleteProgram(SobelProgram);
}
