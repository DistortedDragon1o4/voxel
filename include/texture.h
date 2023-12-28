#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <iostream>
#include <vector>
#include <string>
#include "glad/glad.h"
#include "stb/stb_image.h"

#include "shaderCompiler.h"
#include "chunkDataContainer.h"

namespace std {
    typedef basic_string<unsigned char> ustring;
}

class Texture {
public:
    std::string assets = "assets/";
    GLuint ID;
    GLenum type;
    Texture(const char* imagePath, GLenum texType, GLenum slot, GLenum format, GLenum pixelType, std::string path);

    void TexUnit(Shader& shader, const char* uniform, GLuint unit);
    void Bind();
    void Unbind();
    void Delete();
};

class TextureArray {
public:
    std::string assets = "assets/";
    GLuint ID;
    GLenum type;
    int slot;
    TextureArray(int slot, std::string directory, int start, int stop, std::string path);

    void TexUnit(Shader& shader, const char* uniform, GLuint unit);
    void Bind();
    void Unbind();
    void Delete();
};

struct Sampler {
    unsigned int ID;

    Sampler();

    void bind(int slot);
    void unbind();
    void del();
};

#endif
