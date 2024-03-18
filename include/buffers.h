#ifndef BUFFERS_CLASS_H
#define BUFFERS_CLASS_H

#include "glad/glad.h"
#include <vector>
#include <array>
#include <coordinateContainers.h>

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

struct UnifiedGLBufferContainer {
    ~UnifiedGLBufferContainer() {del();};

    unsigned int ID;
    unsigned int size;
    void* persistentMappedPtr;

    void allocate(unsigned long size, GLenum flags);
    void upload(void* data, unsigned long size, unsigned long byteIndex);

    void map(GLenum flags);
    void map(unsigned long index, unsigned long _size, GLenum flags);

    void bind(GLenum type);
    void bindBufferBase(GLenum type, unsigned int bindIndex);
    void unbind(GLenum type);

    void create();
    void del();
};

#endif