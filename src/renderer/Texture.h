#pragma once
#include <glad/glad.h>
#include <stb_image.h>
#include <iostream>
#include <string>

namespace Renderer {
class Texture {
public:
    unsigned int ID;
    int width, height, nrChannels;
    Texture(const char* cPath) {
        glCreateTextures(GL_TEXTURE_2D, 1, &ID);

        glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_set_flip_vertically_on_load(false);

        unsigned char* cData = stbi_load(cPath, &width, &height, &nrChannels, 0);
        if (cData) {
            GLenum iInternalFormat = (nrChannels == 4) ? GL_RGBA8 : GL_RGB8;
            GLenum iDataFormat = (nrChannels == 4) ? GL_RGBA : GL_RGB;

            glTextureStorage2D(ID, 1, iInternalFormat, width, height);
            glTextureSubImage2D(ID, 0, 0, 0, width, height, iDataFormat, GL_UNSIGNED_BYTE, cData);
            glGenerateTextureMipmap(ID);
            stbi_image_free(cData);

        } else {
            std::cout << "ERROR::TEXTURE::FAILED_TO_LOAD_PATH" << std::endl;
            glTextureStorage2D(ID, 1, GL_RGBA8, 1, 1);
            unsigned char debugColor[] = {255, 0, 255, 255};  // Bright Pink
            glTextureSubImage2D(ID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, debugColor);
        }
    }

    void Bind(unsigned int iSlot) const { glBindTextureUnit(iSlot, ID); }
};
}  // namespace Renderer