#include "../include/texture.h"
#include "chunkList.h"

Texture::Texture(const char* imagePath, GLenum texType, GLenum slot, GLenum format, GLenum pixelType, std::string path) {
    type = texType;

    std::string image = imagePath;
    image = path + assets + image;

    int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load(image.c_str(), &widthImg, &heightImg, &numColCh, 0);

	if (!bytes) {
		std::cout << "Failed loading image successfully\n";
	}

	GLuint texture;
	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(texType, ID);

	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(texType, 0, GL_RGBA, widthImg, heightImg, 0, format, pixelType, bytes);
	glGenerateMipmap(texType);

	stbi_image_free(bytes);
	glBindTexture(texType, 0);
}

void Texture::TexUnit(Shader& shader, const char* uniform, GLuint unit) {
    GLuint tex = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(tex, unit);
}

void Texture::Bind() {
	glBindTexture(type, ID);
}

void Texture::Unbind() {
	glBindTexture(type, 0);
}

void Texture::Delete() {
	glDeleteTextures(1, &ID);
}


TextureArray::TextureArray(GLenum slot, std::string directory, int start, int stop, std::string path) {
	std::ustring imgData;
	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);

	for (int i = start; i < stop; i++) {
		std::ustring data = stbi_load((path + assets + directory + std::to_string(i) + ".png").c_str(), &widthImg, &heightImg, &numColCh, 4);
		
		if(stbi_failure_reason())
    		std::cout << stbi_failure_reason() << "\n";

		// std::cout << "Image loaded: " << imgData.size() << "\n";
		// std::cout << "width: " << widthImg << "\n";
		// std::cout << "height: " << heightImg << "\n";
		imgData.append(data);
	}

	glGenTextures(1, &ID);
	glActiveTexture(slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, widthImg, heightImg, (stop - start), 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData.c_str());

	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::TexUnit(Shader& shader, const char* uniform, GLuint unit) {
    GLuint tex = glGetUniformLocation(shader.ID, uniform);
	shader.Activate();
	glUniform1i(tex, unit);
}

void TextureArray::Bind() {
	glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
}

void TextureArray::Unbind() {
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void TextureArray::Delete() {
	glDeleteTextures(1, &ID);
}
