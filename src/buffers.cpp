#include "../include/buffers.h"

void EBO::Gen(std::vector<GLuint> indices) {
   	glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);
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
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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