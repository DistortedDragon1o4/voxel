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

class UBO {
public:
    GLuint ID;

    UBO(int bindIndex);
    ~UBO();
    
    void Upload(std::array<ChunkProperties, 64> &data);
    void Bind();
    void Unbind();
};

// New stuff

struct newVBO {
    unsigned int ID;

    int numVertices;

    void create();
    void upload(std::vector<unsigned int> &vertices);

    void reAlloc(int numVertices);
    void alloc(int numVertices);
    void subUpload(int offset, std::vector<unsigned int> &vertices);
    void shift(unsigned int buffer, int offset, int size, int shiftBy);

    void bind();
    void unbind();
    void del();
};

struct newIBO {
    unsigned int ID;

    void create();
    void upload(std::vector<unsigned int> &indices);
    void bind();
    void unbind();
    void del();
};


struct UnifiedGLBufferContainer {
    ~UnifiedGLBufferContainer() {del();};

    unsigned int ID;
    unsigned int size;

    void allocate(unsigned long size);
    void upload(void* data, unsigned long size, unsigned long byteIndex);

    void bind(GLenum type);
    void bindBufferBase(GLenum type, unsigned int bindIndex);
    void unbind(GLenum type);

    void create();
    void del();
};

#endif