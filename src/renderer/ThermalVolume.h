#pragma once
#include <glad/glad.h>

namespace Renderer {
class ThermalVolume {
public:
    unsigned int ID;
    int iSizeX, iSizeY, iSizeZ;

    ThermalVolume(int iWidth, int iHeight, int iDepth) : iSizeX(iWidth), iSizeY(iHeight), iSizeZ(iDepth){
        glCreateTextures(GL_TEXTURE_3D, 1, &ID);
        glTextureParameteri(ID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(ID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(ID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureStorage3D(ID, 1, GL_R32F, iSizeX, iSizeY, iSizeZ);
    }

    ~ThermalVolume() { 
        glDeleteTextures(1, &ID);
    }
    
    ThermalVolume(const ThermalVolume& obj) = delete;
    ThermalVolume& operator=(const ThermalVolume&) = delete;

    void Update(const float* pfData) const {
        glTextureSubImage3D(ID, 0, 0, 0, 0, iSizeX, iSizeY, iSizeZ, GL_RED, GL_FLOAT, pfData);
        /*GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "[GL ERROR in ThermalVolume::Update]: " << err << std::endl;
        }*/
    }

    void Bind(unsigned int iSlot) const { 
        glBindTextureUnit(iSlot, ID);
    }
private:
};

} //namespace Renderer