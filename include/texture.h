#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "stb/stb_image.h"

#include "shaderCompiler.h"
#include "chunkDataContainer.h"

namespace std {
    typedef basic_string<unsigned char> ustring;
}

struct Texture {
    unsigned int ID;
    GLenum type;
    int slot;

    unsigned int width;
    unsigned int height;

    void genTexture2d(unsigned int _width, unsigned int _height, GLenum internalFormat);
    void genTextureArray2d(unsigned int _width, unsigned int _height, unsigned int _numLayers, GLenum internalFormat);

    void texUnit(Shader& shader, const char* uniform);

    void bind();
    void unbind();

    void create(GLenum _type, int _slot);
    void del();
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

struct Framebuffer {
    unsigned int ID;

    void bindTexture(Texture &texture);

    void bind();
    void unbind();

    void create();
    void del();
};
