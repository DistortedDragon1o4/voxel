#ifndef SHADER_COMPILER_H
#define SHADER_COMPILER_H

#include <glad/glad.h>
#include <iostream>

class Shader {
public:
    GLuint ID;
    Shader(const char* vertexSource, const char* fragmentSource);
    Shader(const char* vertexSource, const char* fragmentSource, const char* geometrySource);

    void Activate();
    void Delete();

    void compileErrors(unsigned int shader, const char* type);
};

class Compute {
public:
    GLuint ID;

    void Build(const char* computeSource);
    void Activate();
    void Delete();
    void Dispatch();
    void Wait();

    void compileErrors(unsigned int shader, const char* type);
};

#endif