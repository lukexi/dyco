#include "gl.h"

void InitGLEW() {
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        printf("Could not init glew.\n");
        exit(1);
    }
    GLenum GLEWError = glGetError();
    if (GLEWError) {
        printf("GLEW returned error: %i\n", GLEWError);
    }
}

void GLCheck(const char* name) {
    GLenum Error = glGetError();
    if (Error != GL_NO_ERROR) {
        printf("%s: ", name);
        switch (Error) {
            case GL_INVALID_ENUM: printf("Invalid enum\n");
                break;
            case GL_INVALID_VALUE: printf("Invalid value\n");
                break;
            case GL_INVALID_OPERATION: printf("Invalid operation\n");
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: printf("Invalid framebuffer op\n");
                break;
            case GL_OUT_OF_MEMORY: printf("GL Out of memory\n");
                break;
        }
        exit(1);
    }
}

void GLAPIENTRY DebugCallback(GLenum Source, GLenum Type,
    GLuint ID, GLenum Severity, GLsizei Length,
    const GLchar* Message, const void* UserParam) {
    printf("===OPENGL DEBUG 0x%X: %s\n", ID, Message);
}

// Turns on basic OpenGL 4.3 Debugging output.
// Lots more fancy options available,
// like synchronous output, debug groups,
// object naming, see here page 65:
// https://www.slideshare.net/Mark_Kilgard/opengl-45-update-for-nvidia-gpus
void EnableGLDebug() {
    glDebugMessageCallback(DebugCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE,
        GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    glEnable(GL_DEBUG_OUTPUT);
}


SDL_Window* CreateWindow(const char* Name, int X, int Y, int Width, int Height) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* Window = SDL_CreateWindow(Name, X,Y, Width,Height, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GLContext GLContext = SDL_GL_CreateContext(Window);
    SDL_GL_MakeCurrent(Window, GLContext);
    InitGLEW();
    // EnableGLDebug();

    return Window;
}

void SwapWindowQ(SDL_Window* Window) {
    SDL_GL_SwapWindow(Window);
    SDL_Event Event;
    while (SDL_PollEvent(&Event)) {
        if (Event.type == SDL_QUIT) exit(0);
    }
}
