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

void SSBO::Gen(int bindIndex, int size) {
   	glGenBuffers(1, &ID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    totalSize = size;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindIndex, ID);
}

void SSBO::Upload(std::vector<float> data, int offset, int size, int bindIndex) {
   	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, &data[0]);
	//glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindIndex, ID, offset, size);
	// float* ssboData = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	// for (int i = offset / sizeof(float); i < (offset + size) / sizeof(float); i++) {
	// 	ssboData[i] = data.at(i - (offset / sizeof(float)));
	// }
	// glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::Upload(std::vector<int> data, int offset, int size, int bindIndex) {
   	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, &data[0]);
	//glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindIndex, ID, offset, size);
	// float* ssboData = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	// for (int i = offset / sizeof(float); i < (offset + size) / sizeof(float); i++) {
	// 	ssboData[i] = data.at(i - (offset / sizeof(float)));
	// }
	// glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::Bind() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
}

void SSBO::Unbind() {
  	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SSBO::Delete() {
    glDeleteBuffers(1, &ID);
}

UBO::UBO(int bindIndex) {
	glGenBuffers(1, &ID);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindIndex, ID);
}

void UBO::Upload(std::array<ChunkProperties, 64> &data) {
	glNamedBufferData(ID, 64 * sizeof(ChunkProperties), &data[0], GL_STREAM_DRAW);
}

void UBO::Bind() {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
}

void UBO::Unbind() {
  	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UBO::~UBO() {
    glDeleteBuffers(1, &ID);
}




void newVBO::create() {
	glCreateBuffers(1, &ID);
}

void newVBO::upload(std::vector<unsigned int> &vertices) {
	glNamedBufferData(ID, vertices.size() * sizeof(unsigned int), &vertices[0], GL_DYNAMIC_DRAW);
}

void newVBO::alloc(int numVertices) {
	this->numVertices = numVertices;
	glNamedBufferStorage(ID, numVertices * 2 * sizeof(unsigned int), NULL, GL_DYNAMIC_STORAGE_BIT);
}

void newVBO::reAlloc(int numVertices) {
	this->numVertices = numVertices;
	unsigned int crntID;
	glCreateBuffers(1, &crntID);
	glNamedBufferStorage(crntID, numVertices * 2 * sizeof(unsigned int), NULL, GL_DYNAMIC_STORAGE_BIT);
	glCopyBufferSubData(ID, crntID, 0, 0, this->numVertices);
	glClientWaitSync(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0), GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	del();
	ID = crntID;
}

void newVBO::subUpload(int offset, std::vector<unsigned int> &vertices) {
	std::cout << "I run\n";
	glNamedBufferSubData(ID, offset * 2 * sizeof(unsigned int), vertices.size() * sizeof(unsigned int), &vertices[0]);
}

void newVBO::shift(unsigned int buffer, int offset, int size, int shiftBy) {
	if (shiftBy == 0)
		return;
	glCopyNamedBufferSubData(ID, buffer, offset * 2 * sizeof(unsigned int), 0, size * 2 * sizeof(unsigned int));
	glCopyNamedBufferSubData(buffer, ID, 0, (offset + shiftBy) * 2 * sizeof(unsigned int), size * 2 * sizeof(unsigned int));
}

void newVBO::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void newVBO::unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void newVBO::del() {
	glDeleteBuffers(1, &ID);
}


void newIBO::create() {
	glCreateBuffers(1, &ID);
}

void newIBO::upload(std::vector<unsigned int> &indices) {
	glNamedBufferData(ID, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);
}

void newIBO::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void newIBO::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void newIBO::del() {
	glDeleteBuffers(1, &ID);
}