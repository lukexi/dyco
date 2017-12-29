#include "shader.h"
#include <stdio.h>
#include <stdlib.h>


char* ReadFile(const char *FilePath) {

    // Get file handle
    FILE *FileHandle = fopen(FilePath, "rb");
    if (!FileHandle) {
        printf("Couldn't open file %s\n", FilePath);
        exit(1);
        return NULL;
    }

    // Find file size
    fseek(FileHandle, 0, SEEK_END);
    long Length = ftell(FileHandle);

    // Read into buffer
    char *Buffer = (char*)malloc(Length+1);
    fseek(FileHandle, 0, SEEK_SET);
    fread(Buffer, Length, 1, FileHandle);
    fclose(FileHandle);

    // Null terminate
    Buffer[Length] = 0;

    return Buffer;
}


GLuint CreateShader(GLenum ShaderType, const char* ShaderSource) {
    GLuint Shader = glCreateShader(ShaderType);

    glShaderSource(Shader, 1, (const GLchar**)&ShaderSource, 0);

    glCompileShader(Shader);

    int IsVertCompiled;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &IsVertCompiled);
    if(IsVertCompiled == GL_FALSE) {
        int MaxLength;
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &MaxLength);
        if (!MaxLength) return 0;

        char *InfoLog = (char *)calloc(1, MaxLength);

        glGetShaderInfoLog(Shader, MaxLength, &MaxLength, InfoLog);

        printf("%s\n", InfoLog);

        free(InfoLog);
        return 0;
    }
    return Shader;
}

int LinkProgram(GLuint ShaderProgram) {
    glLinkProgram(ShaderProgram);

    int IsLinked;
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, (int *)&IsLinked);
    if(IsLinked == GL_FALSE) {
        int MaxLength;
        glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &MaxLength);
        if (!MaxLength) return 0;

        char *InfoLog = (char *)calloc(1, MaxLength);

        glGetProgramInfoLog(ShaderProgram, MaxLength, &MaxLength, InfoLog);

        printf("%s\n", InfoLog);

        free(InfoLog);
        return 0;
    }

    return 1;
}

GLuint CreateVertFragProgram(const char* VertSource, const char* FragSource) {

    if (VertSource == NULL || FragSource == NULL) {
      printf("Called CreateVertFragProgram with NULL VertSource and/or FragSource\n");
      // exit(1);
    }

    GLuint VertShader = CreateShader(GL_VERTEX_SHADER, VertSource);
    GLuint FragShader = CreateShader(GL_FRAGMENT_SHADER, FragSource);

    if (VertShader == 0 || FragShader == 0) {
      printf("Couldn't create shader\n");
      // exit(1);
    }

    // Combine the vert and frag shaders into a single shader program
    GLuint ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertShader);
    glAttachShader(ShaderProgram, FragShader);

    // Link the program
    if (!LinkProgram(ShaderProgram)) {
        // exit(1);
    }

    // Delete shader components that are now embedded in the program
    glDeleteShader(VertShader);
    glDeleteShader(FragShader);

    // Set it as active
    glUseProgram(ShaderProgram);

    return ShaderProgram;
}


GLuint CreateVertFragProgramFromPath(const char* VertPath, const char* FragPath) {
    const char* VertSource = ReadFile(VertPath);
    const char* FragSource = ReadFile(FragPath);
    GLuint Program = CreateVertFragProgram(
        VertSource,
        FragSource
        );
    free((void*)VertSource);
    free((void*)FragSource);
    return Program;

}

