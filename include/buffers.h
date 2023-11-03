#ifndef BUFFERS_CLASS_H
#define BUFFERS_CLASS_H

#include "glad/glad.h"
#include <vector>

class EBO {
public:
    GLuint ID;

    void Gen(std::vector<GLuint> indices);
    void Bind();
    void Unbind();
    void Delete();
};

class VBO {
public:
    GLuint ID;
    
    void Gen(std::vector<int> vertices);
    void Gen(std::vector<unsigned int> vertices);
    void Gen(std::vector<short> vertices);
    void Gen(std::vector<float> vertices);
    void Bind();
    void Unbind();
    void Delete();
};

class SSBO {
public:
    int totalSize;
    GLuint ID;
    
    void Gen(int bindIndex, int size);
    void Upload(std::vector<float> data, int offset, int size, int bindIndex);
    void Upload(std::vector<int> data, int offset, int size, int bindIndex);
    void Bind();
    void Unbind();
    void Delete();
};

#endif