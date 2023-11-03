#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "buffers.h"

class VAO {
public:
    GLuint ID;
    VAO();

    void LinkAttribIPointer(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
    void LinkAttribPointer(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

    void LinkAttribI3iv(GLuint layout, glm::ivec3 value);

    void Bind();
    void Unbind();
    void Delete();
};

#endif