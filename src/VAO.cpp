#include "../include/VAO.h"

VAO::VAO() {
    glGenVertexArrays(1, &ID);
}

void VAO::LinkAttribIPointer(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    VBO.Bind();
    glVertexAttribIPointer(layout, numComponents, type, stride, offset);
	glEnableVertexAttribArray(layout);
    VBO.Unbind();
}

void VAO::LinkAttribI3iv(GLuint layout, glm::ivec3 value) {
    glVertexAttribI3iv(layout, &value[0]);
}

void VAO::LinkAttribPointer(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    VBO.Bind();
    glVertexAttribPointer(layout, numComponents, type, GL_TRUE, stride, offset);
	glEnableVertexAttribArray(layout);
    VBO.Unbind();
}

void VAO::Bind() {
    glBindVertexArray(ID);
}

void VAO::Unbind() {
    glBindVertexArray(0);
}

void VAO::Delete() {
    glDeleteVertexArrays(1, &ID);
}




void newVAO::create() {
    glCreateVertexArrays(1, &ID);
}

void newVAO::bind() {
    glBindVertexArray(ID);
}

void newVAO::unbind() {
    glBindVertexArray(0);
}

void newVAO::del() {
    glDeleteVertexArrays(1, &ID);
}
