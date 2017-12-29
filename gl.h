#ifndef GL_H
#define GL_H
#include <SDL2/SDL.h>
#include <GL/glew.h>

SDL_Window* CreateWindow(const char* Name, int X, int Y, int Width, int Height);

void GLCheck(const char* Name);

// Handles the quit event. Don't use this if you need events.
void SwapWindowQ(SDL_Window* Window);

#endif // GL_H
