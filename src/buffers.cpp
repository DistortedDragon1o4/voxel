#include "../include/buffers.h"
#include "coordinateContainers.h"
#include <iostream>


void UnifiedGLBufferContainer::create() {
	glCreateBuffers(1, &ID);
}

void UnifiedGLBufferContainer::del() {
	glDeleteBuffers(1, &ID);
}

void UnifiedGLBufferContainer::bind(GLenum type) {
	glBindBuffer(type, ID);
}

void UnifiedGLBufferContainer::bindBufferBase(GLenum type, unsigned int bindIndex) {
	glBindBufferBase(type, bindIndex, ID);
}

void UnifiedGLBufferContainer::unbind(GLenum type) {
	glBindBuffer(type, ID);
}

void UnifiedGLBufferContainer::allocate(unsigned long size, GLenum flags) {
	this->size = size;
	glNamedBufferStorage(ID, size, NULL, flags);
}

void UnifiedGLBufferContainer::upload(void* data, unsigned long size, unsigned long byteIndex) {
	glNamedBufferSubData(ID, byteIndex, size, data);
}

void UnifiedGLBufferContainer::map(GLenum flags) {
	persistentMappedPtr = glMapNamedBufferRange(ID, 0, size, flags);
}

void UnifiedGLBufferContainer::map(unsigned long index, unsigned long _size, GLenum flags) {
	persistentMappedPtr = glMapNamedBufferRange(ID, index, _size, flags);
}




















void EBO::Gen(std::vector<GLuint> indices) {
   	glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_DYNAMIC_DRAW);
}

void EBO::Bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::Unbind() {
  	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::Delete() {
    glDeleteBuffers(1, &ID);
}

void VBO::Gen(std::vector<int> vertices) {
   	glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(int), &vertices[0], GL_DYNAMIC_DRAW);
}

void VBO::Gen(std::vector<unsigned int> vertices) {
   	glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(int), &vertices[0], GL_DYNAMIC_DRAW);
}

void VBO::Gen(std::vector<short> vertices) {
   	glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(short), &vertices[0], GL_DYNAMIC_DRAW);
}

void VBO::Gen(std::vector<float> vertices) {
   	glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);
}

void VBO::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind() {
  	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete() {
    glDeleteBuffers(1, &ID);
}