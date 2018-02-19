#include "gl.h"
#include "nanovg.h"
#include "stb_image.h"
#include "framebuffer.h"
#include "quad.h"
#include "shader.h"
#include "utils.h"
#define NANOVG_GL3
#include "nanovg_gl.h"
#include <sys/stat.h>
#include <stdbool.h>


typedef struct {
    float X;
    float Y;
    float Z;
} vec3;

typedef struct {
    int X;
    int Y;
} vec2;

typedef struct {
    vec2 Points[1024];
    int Index;
} contour;


int ImageWidth, ImageHeight, ImageChannels;
unsigned char* ImageData;
unsigned char* SobelData = NULL;
int OriginalImage;
int SobelImage;
int PostContourImage;

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
    bool A = LoadShader(&SobelProgram,   &SobelProgramMod,   QuadVertSource, "cv-sobel.frag");
    bool B = LoadShader(&ThreshProgram,  &ThreshProgramMod,  QuadVertSource, "cv-thresh.frag");
    return A||B;
}

void DoPass(GLuint InputTex, GLuint Program, GLuint OutputFB) {
    glUseProgram(Program);
    glBindTexture(GL_TEXTURE_2D, InputTex);
    glBindFramebuffer(GL_FRAMEBUFFER, OutputFB);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(QuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Setup(NVGcontext* NVG) {
    ImageData = stbi_load("IMG_3169.jpg",
        &ImageWidth, &ImageHeight, &ImageChannels, 4);
    OriginalImage = nvgCreateImageRGBA(NVG, ImageWidth, ImageHeight, 0, ImageData);

    QuadVertSource = ReadFile("quad.vert");
    QuadVAO = CreateQuad(FullscreenQuadVertices);

    InitFramebuffer(ImageWidth, ImageHeight, &SobelFB);
    InitFramebuffer(ImageWidth, ImageHeight, &ThreshFB);
}



vec3 Pixel(vec2 Loc, unsigned char* Image) {
    int X = Loc.X; int Y = Loc.Y;
    unsigned char* Pixel = &Image[((Y*ImageWidth)+X)*4];
    return (vec3){.X=Pixel[0]/255.0, .Y=Pixel[1]/255.0, .Z=Pixel[2]/255.0};
}

void ClearPixel(vec2 Loc, unsigned char* Image) {
    int X = Loc.X; int Y = Loc.Y;
    unsigned char* Pixel = &Image[((Y*ImageWidth)+X)*4];
    Pixel[0] = 0; Pixel[1] = 0; Pixel[2] = 0;
}

float AvgV3(vec3 Vec) {
    return (Vec.X+Vec.Y+Vec.Z) / 3.0;
}

bool CheckPixel(vec2 Loc, unsigned char* Image) {
    return AvgV3(Pixel(Loc, Image)) > 0.5;
}

vec2 MulV2(vec2 A, int B) {
    return (vec2){A.X*B, A.Y*B};
}

vec2 AddV2(vec2 A, vec2 B) {
    return (vec2){A.X+B.X, A.Y+B.Y};
}

bool EqV2(vec2 A, vec2 B) {
    return A.X == B.X && A.Y == B.Y;
}

void FollowContour(vec2 StartXY, unsigned char* Image, contour* Contour) {
    if (!CheckPixel(StartXY, Image)) return;

    const vec2 Directions[] = {
        {.X=  0, .Y=  1}, // N
        {.X=  1, .Y=  1}, // NE
        {.X=  1, .Y=  0}, // E
        {.X=  1, .Y= -1}, // SE
        {.X=  0, .Y= -1}, // S
        {.X= -1, .Y= -1}, // SW
        {.X= -1, .Y=  0}, // W
        {.X= -1, .Y=  1}, // NW
    };
    const int Radius = 2;

    vec2 CurPosition = StartXY;
    vec2 CurDirection = {0,0};
    bool FoundNext = true;
    while (FoundNext) {
        float BestSum = 0;
        vec2 BestDirection = {0,0};

        for (int I = 0; I < ARRAY_LEN(Directions); I++) {
            vec2 Direction = Directions[I];

            // Don't go backwards
            if (EqV2(Direction,
                MulV2(CurDirection, -1))) continue;

            float Sum = 0;
            for (int R = 1; R <= Radius; R++) {
                // Sum the values along this direction vector
                float Val = AvgV3(
                    Pixel(
                        AddV2(CurPosition,
                            MulV2(Direction, R)),
                    Image));
                // Break if we've reached a trough.
                // if (Val < 0.3) { break; }
                Sum += Val;
            }
            if (Sum > BestSum) {
                BestSum       = Sum;
                BestDirection = Direction;
            }
        }

        // Add the current point
        if (Contour->Index >= ARRAY_LEN(Contour->Points)) {
            printf("Found a contour longer than we have space for!\n");
            break;
        }
        Contour->Points[Contour->Index++] = CurPosition;
        ClearPixel(CurPosition, Image);

        if (BestSum > 0) {
            // printf("Continuing in direction %i %i\n", BestDirection.X, BestDirection.Y);
            CurDirection = BestDirection;
            CurPosition = AddV2(CurPosition, BestDirection);
        } else {
            FoundNext = false;
        }
    }
}


contour Contours[512];
int NumContours = 0;

void FindContoursInImage(unsigned char* Image) {
    const int MinContourLength = 20;
    for (int X = 0; X < 1024; X++) {
        for (int Y = 0; Y < 768; Y++) {
            if (NumContours >= ARRAY_LEN(Contours)) break;
            vec2 StartXY = {X,Y};

            contour* Contour = &Contours[NumContours];
            Contour->Index = 0;

            FollowContour(StartXY, SobelData, Contour);

            if (Contour->Index > MinContourLength) NumContours++;
        }
    }
}

double SawWave(double Freq) {
    double Now = (double)SDL_GetTicks()/1000.0;
    double Frac;
    return modf(Now * Freq, &Frac);
}

void DrawFoundContours(NVGcontext* NVG) {
    double Saw = SawWave(0.1);

    int DrawIndex = Saw * NumContours;

    for (int C = 0; C < NumContours; C++) {
        if (C != DrawIndex) continue;
        contour* Contour = &Contours[C];

        float Hue = (float)C / (float)NumContours;
        nvgBeginPath(NVG);
        for (int I = 0; I < Contour->Index; I++) {
            if (I == 0) nvgMoveTo(NVG, Contour->Points[I].X, Contour->Points[I].Y);
            else        nvgLineTo(NVG, Contour->Points[I].X, Contour->Points[I].Y);
        }
        nvgStrokeColor(NVG, nvgHSL(Hue,0.8,0.6));
        nvgStrokeWidth(NVG, 2);
        nvgStroke(NVG);
    }
}

void DrawNVGImage(NVGcontext* NVG, int Image, float Alpha) {
    nvgBeginPath(NVG);
    NVGpaint ImagePaint = nvgImagePattern(NVG, 0,0, ImageWidth, ImageHeight, 0, Image, Alpha);
    nvgRect(NVG, 0,0,ImageWidth,ImageHeight);
    nvgFillPaint(NVG, ImagePaint);
    nvgFill(NVG);
}

void Tick(SDL_Window* Window, NVGcontext* NVG) {
    if (LoadShaders()) {

        GLuint InputTex = nvglImageHandleGL3(NVG, OriginalImage);
        DoPass(InputTex, SobelProgram,   SobelFB.Framebuffer);
        InputTex = SobelFB.FramebufferTex;
        DoPass(InputTex, ThreshProgram,  ThreshFB.Framebuffer);

        // Dump the sobel-ized image into a CPU buffer & NVG image
        free(SobelData);
        nvgDeleteImage(NVG,SobelImage);
        SobelData = calloc(1, sizeof(unsigned char) * ImageWidth * ImageHeight * 4);
        glBindTexture(GL_TEXTURE_2D, ThreshFB.FramebufferTex);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, SobelData);
        SobelImage = nvgCreateImageRGBA(NVG, ImageWidth, ImageHeight, 0, SobelData);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        NEWTIME(FindContours);
        FindContoursInImage(SobelData);
        ENDTIME(FindContours);

        // Get a second image after the FindContours algorithm has eaten through the image
        PostContourImage = nvgCreateImageRGBA(NVG, ImageWidth, ImageHeight, 0, SobelData);

        // printf("Found contour with points %i\n", Contour.Index);
    }
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    nvgBeginFrame(NVG, 1024,768,2);


    DrawNVGImage(NVG, OriginalImage, 0.5);
    // DrawNVGImage(NVG, SobelImage, 0.5);
    // DrawNVGImage(NVG, PostContourImage, 1);

    // Draw the found contours
    DrawFoundContours(NVG);

    // Test pixel extraction
    // double Frac;
    // double Saw = modf((double)SDL_GetTicks()/1000.0/2, &Frac);
    // for (int I = 0; I<10; I++) {
    //     int X = Saw*1024;
    //     int Y = 100*I;
    //     NVGcolor Color = Pixel(X,Y, SobelData);
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
    nvgDeleteImage(NVG, OriginalImage);
    nvgDeleteImage(NVG, SobelImage);
    nvgDeleteImage(NVG, PostContourImage);
    DeleteFramebuffer(SobelFB);
    glDeleteProgram(SobelProgram);
}
