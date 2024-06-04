#include "../include/texture.h"

void Texture::genTexture2d(unsigned int _width, unsigned int _height, GLenum internalFormat) {
	if (type != GL_TEXTURE_2D)
		return;

	width = _width;
	height = _height;

	glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureStorage2D(ID, 1, internalFormat, width, height);
}

void Texture::genTextureArray2d(unsigned int _width, unsigned int _height, unsigned int _numLayers, GLenum internalFormat) {
	if (type != GL_TEXTURE_2D_ARRAY)
		return;

	width = _width;
	height = _height;

	glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureStorage3D(ID, 1, internalFormat, width, height, _numLayers);
}

void Texture::texUnit(Shader& shader, const char* uniform) {
    GLuint tex = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(tex, slot);
}

void Texture::bind() {
	glBindTextureUnit(slot, ID);
}

void Texture::unbind() {
	glBindTextureUnit(slot, 0);
}

void Texture::create(GLenum _type, int _slot) {
	type = _type;
	slot = _slot;
	glCreateTextures(type, 1, &ID);
	bind();
}

void Texture::del() {
	glDeleteTextures(1, &ID);
}



TextureArray::TextureArray(int slot, std::string directory, int start, int stop, std::string path) {
	slot = slot;
	std::ustring imgData;
	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);

	for (int i = start; i <= stop; i++) {
		std::ustring data = stbi_load((path + assets + directory + std::to_string(i) + ".png").c_str(), &widthImg, &heightImg, &numColCh, 4);
		
		if(stbi_failure_reason())
    		std::cout << stbi_failure_reason() << "\n";

		// std::cout << "Image loaded: " << imgData.size() << "\n";
		// std::cout << "width: " << widthImg << "\n";
		// std::cout << "height: " << heightImg << "\n";
		imgData.append(data);
	}

	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &ID);
	glBindTextureUnit(slot, ID);

	glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTextureStorage3D(ID, 1, GL_RGBA8, widthImg, heightImg, (stop - start));
	glTextureSubImage3D(ID, 0, 0, 0, 0, widthImg, heightImg, (stop - start), GL_RGBA, GL_UNSIGNED_BYTE, imgData.c_str());

	// glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	// glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);

	glTextureParameteri(ID, GL_TEXTURE_MAX_LEVEL, 4);
	glTextureParameteri(ID, GL_TEXTURE_BASE_LEVEL, 2);

	glGenerateTextureMipmap(ID);

	glTextureParameterf(ID, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

	// glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::TexUnit(Shader& shader, const char* uniform, GLuint unit) {
    GLuint tex = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(tex, unit);
}

void TextureArray::Bind() {
	// glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
	glBindTextureUnit(slot, ID);
}

void TextureArray::Unbind() {
	// glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindTextureUnit(slot, 0);
}

void TextureArray::Delete() {
	glDeleteTextures(1, &ID);
}


Sampler::Sampler() {
	glCreateSamplers(1, &ID);

	glSamplerParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glSamplerParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glSamplerParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glSamplerParameterf(ID, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);
}

void Sampler::bind(int slot) {
	glBindSampler(slot, ID);
}

void Sampler::unbind() {
	glBindSampler(0, 0);
}

void Sampler::del() {
	glDeleteSamplers(1, &ID);
}

void Framebuffer::bindTexture(Texture &texture) {
	// bind();
	glNamedFramebufferTexture(ID, GL_DEPTH_ATTACHMENT, texture.ID, 0);

	// glDrawBuffer(GL_NONE);
	// glReadBuffer(GL_NONE);
	// unbind();
}

void Framebuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::create() {
	glCreateFramebuffers(1, &ID);
}

void Framebuffer::del() {
	glDeleteFramebuffers(1, &ID);
}