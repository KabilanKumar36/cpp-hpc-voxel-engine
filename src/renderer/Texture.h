#pragma once
#include <glad/glad.h>
#include <iostream>
#include <string>
#include "../vendor/stb_image.h"

namespace Renderer {
class Texture {
public:
    unsigned int ID;
    int width, height, nrChannels;
    Texture(const char* cPath) {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_set_flip_vertically_on_load(false);

        unsigned char* cData = stbi_load(cPath, &width, &height, &nrChannels, 0);
        if (cData) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

            glTexImage2D(
                GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, cData);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "ERROR::TEXTURE::FAILED_TO_LOAD_PATH" << std::endl;
            unsigned char debugColor[] = {255, 0, 255, 255};  // Bright Pink
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, debugColor);
        }
        stbi_image_free(cData);
    }

    void Bind(unsigned int iSlot) const {
        glActiveTexture(GL_TEXTURE0 + iSlot);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
};
}  // namespace Renderer