#include "gl.h"
#include "nanovg.h"
#include "stb_image.h"

int ImageWidth, ImageHeight, ImageChannels;
unsigned char *ImageData;
int NVGImage;

void Setup(NVGcontext* NVG) {
    ImageData = stbi_load("IMG_3169.jpg",
        &ImageWidth, &ImageHeight, &ImageChannels, 4);
    NVGImage = nvgCreateImageRGBA(NVG, ImageWidth, ImageHeight, 0, ImageData);
}

NVGcolor Pixel(int X, int Y) {
    unsigned char* Pixel = &ImageData[((Y*ImageWidth)+X)*4];
    return nvgRGBA(Pixel[0], Pixel[1], Pixel[2], Pixel[3]);
}

void Tick(SDL_Window* Window, NVGcontext* NVG) {
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    nvgBeginFrame(NVG, 1024,768,2);
    nvgBeginPath(NVG);
    NVGpaint ImagePaint = nvgImagePattern(NVG, 0,0, ImageWidth, ImageHeight, 0, NVGImage, 1);
    nvgRect(NVG, 0,0,ImageWidth,ImageHeight);
    nvgFillPaint(NVG, ImagePaint);
    nvgFill(NVG);

    double Frac;
    double Saw = modf((double)SDL_GetTicks()/1000.0/2, &Frac);
    for (int I = 0; I<10; I++) {
        int X = Saw*1024;
        int Y = 100*I;
        NVGcolor Color = Pixel(X,Y);
        nvgBeginPath(NVG);
        nvgRect(NVG, X,Y,100,100);
        nvgFillColor(NVG, Color);
        nvgFill(NVG);
    }


    nvgEndFrame(NVG);
    SwapWindowQ(Window);
}

void Cleanup(NVGcontext* NVG) {
    stbi_image_free(ImageData);
    nvgDeleteImage(NVG, NVGImage);
}
