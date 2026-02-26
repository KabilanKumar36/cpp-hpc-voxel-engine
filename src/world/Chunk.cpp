#include <cstdlib>  // required for aligned malloc and aligned free

#ifdef _WIN32
#include <malloc.h>
#define ALLOCATE_ALIGNED(size, alignment) _aligned_malloc((size), (alignment))
#define FREE_ALIGNED(ptr) _aligned_free(ptr)
#else
#define ALLOCATE_ALIGNED(size, alignment) std::aligned_alloc((alignment), (size))
#define FREE_ALIGNED(ptr) std::free(ptr)
#endif

#include <algorithm>
#include <cstring>
#include <iostream>
#include "Chunk.h"

//*********************************************************************
Chunk::Chunk(int iX, int iZ) : m_iChunkX(iX), m_iChunkZ(iZ) {
    m_bEnableFaceCulling = true;

    m_pfCurrFrameData = static_cast<float*>(ALLOCATE_ALIGNED(CHUNK_VOL * sizeof(float), 64));
    m_pfNextFrameData = static_cast<float*>(ALLOCATE_ALIGNED(CHUNK_VOL * sizeof(float), 64));
    std::fill_n(m_pfCurrFrameData, CHUNK_VOL, 0.0f);
    std::fill_n(m_pfNextFrameData, CHUNK_VOL, 0.0f);

    updateHeightData();
}

//*********************************************************************
Chunk::~Chunk() {
    if (m_pVAO)
        delete m_pVAO;
    if (m_pVBO)
        delete m_pVBO;
    if (m_pIBO)
        delete m_pIBO;

    FREE_ALIGNED(m_pfCurrFrameData);
    FREE_ALIGNED(m_pfNextFrameData);
}
//*********************************************************************
Chunk::Chunk(Chunk&& other) noexcept
    : m_vec_fVertices(std::move(other.m_vec_fVertices)),
      m_vec_uiIndices(std::move(other.m_vec_uiIndices)),
      m_pVAO(other.m_pVAO),
      m_pVBO(other.m_pVBO),
      m_pIBO(other.m_pIBO),
      m_pfCurrFrameData(other.m_pfCurrFrameData),
      m_pfNextFrameData(other.m_pfNextFrameData),
      m_iChunkX(other.m_iChunkX),
      m_iChunkZ(other.m_iChunkZ),
      m_bEnableFaceCulling(other.m_bEnableFaceCulling) {
    other.m_pVAO = nullptr;
    other.m_pVBO = nullptr;
    other.m_pIBO = nullptr;
    other.m_pfCurrFrameData = nullptr;
    other.m_pfNextFrameData = nullptr;

    std::memcpy(m_iBlocks, other.m_iBlocks, sizeof(m_iBlocks));
    std::memcpy(m_iHeightData, other.m_iHeightData, sizeof(m_iHeightData));

    // Copy neighbours pointers
    for (int i = 0; i < 6; ++i) m_pNeighbours[i] = other.m_pNeighbours[i];
}
//*********************************************************************
Chunk& Chunk::operator=(Chunk&& other) noexcept {
    if (this != &other) {
        m_vec_fVertices = std::move(other.m_vec_fVertices);
        m_vec_uiIndices = std::move(other.m_vec_uiIndices);

        if (m_pVAO)
            delete m_pVAO;
        if (m_pVBO)
            delete m_pVBO;
        if (m_pIBO)
            delete m_pIBO;

        m_pVAO = other.m_pVAO;
        m_pVBO = other.m_pVBO;
        m_pIBO = other.m_pIBO;

        other.m_pVAO = nullptr;
        other.m_pVBO = nullptr;
        other.m_pIBO = nullptr;

        FREE_ALIGNED(m_pfCurrFrameData);
        FREE_ALIGNED(m_pfNextFrameData);

        m_pfCurrFrameData = other.m_pfCurrFrameData;
        m_pfNextFrameData = other.m_pfNextFrameData;
        other.m_pfCurrFrameData = nullptr;
        other.m_pfNextFrameData = nullptr;
        m_iChunkX = other.m_iChunkX;
        m_iChunkZ = other.m_iChunkZ;
        m_bEnableFaceCulling = other.m_bEnableFaceCulling;

        std::memcpy(m_iBlocks, other.m_iBlocks, sizeof(m_iBlocks));
        std::memcpy(m_iHeightData, other.m_iHeightData, sizeof(m_iHeightData));
        for (int i = 0; i < 6; ++i) m_pNeighbours[i] = other.m_pNeighbours[i];
    }
    return *this;
}
//*********************************************************************
AABB Chunk::GetAABB() const {
    float fWorldX = static_cast<float>(m_iChunkX * CHUNK_SIZE);
    float fWorldZ = static_cast<float>(m_iChunkZ * CHUNK_SIZE);

    int iMinHeight = CHUNK_HEIGHT;
    int iMaxHeight = 0;

    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {
            int iHeight = m_iHeightData[iX][iZ];
            if (iHeight < iMinHeight)
                iMinHeight = iHeight;
            if (iHeight > iMaxHeight)
                iMaxHeight = iHeight;
        }
    }

    return AABB(
        Core::Vec3(fWorldX, static_cast<float>(iMinHeight), fWorldZ),
        Core::Vec3(fWorldX + CHUNK_SIZE, static_cast<float>(iMaxHeight + 1), fWorldZ + CHUNK_SIZE));
}

//*********************************************************************
void Chunk::SetBlockAt(int iX, int iY, int iZ, uint8_t uiBlockType) {
    int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
    if (iIndex != -1) {
        m_iBlocks[iIndex] = uiBlockType;
    }
}

//*********************************************************************
void Chunk::updateHeightData() {
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFrequency(0.04f);
    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {
            int iWorldX = (m_iChunkX * CHUNK_SIZE) + iX;
            int iWorldZ = (m_iChunkZ * CHUNK_SIZE) + iZ;

            float fNoiseVal =
                noise.GetNoise(static_cast<float>(iWorldX), static_cast<float>(iWorldZ));

            // Map -1..1 to Height
            int iHeight = static_cast<int>((fNoiseVal + 1.0f) * 10.0f);

            // Clamp height
            iHeight = std::clamp(iHeight, 0, CHUNK_HEIGHT - 1);
            m_iHeightData[iX][iZ] = iHeight;

            for (int iY = 0; iY <= iHeight; iY++) {
                int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
                if (iIndex == -1)
                    continue;

                uint8_t iBlockType = STONE;
                if (iY == iHeight)
                    iBlockType = GRASS;
                else if (iY > iHeight - 3)
                    iBlockType = DIRT;

                m_iBlocks[iIndex] = iBlockType;
            }
        }
    }
}

//*********************************************************************
void Chunk::updateBuffers() {
    if (m_pVBO) {
        delete m_pVBO;
        m_pVBO = nullptr;
    }
    if (m_pIBO) {
        delete m_pIBO;
        m_pIBO = nullptr;
    }

    if (!m_vec_fVertices.empty()) {
        m_pVBO = new Renderer::VertexBuffer(
            m_vec_fVertices.data(),
            static_cast<unsigned int>(m_vec_fVertices.size()) * sizeof(float));
    }

    if (!m_vec_uiIndices.empty()) {
        m_pIBO = new Renderer::IndexBuffer(m_vec_uiIndices.data(),
                                           static_cast<unsigned int>(m_vec_uiIndices.size()));
    }

    if (m_pVBO) {
        // Attribute 0: Pos (3 floats)
        m_pVAO->LinkAttribute(*m_pVBO, 0, 3, 5, 0);
        // Attribute 1: TexCoord (2 floats)
        m_pVAO->LinkAttribute(*m_pVBO, 1, 2, 5, 3);
        if (m_pIBO) {
            m_pVAO->AttachIndexBuffer(*m_pIBO);
        }
    }
}

//*********************************************************************
void Chunk::ReconstructMesh() {
    m_vec_fVertices.clear();
    m_vec_uiIndices.clear();
    for (int iX = 0; iX < CHUNK_SIZE; iX++) {
        for (int iZ = 0; iZ < CHUNK_SIZE; iZ++) {
            for (int iY = 0; iY < CHUNK_HEIGHT; iY++) {
                int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
                if (iIndex == -1)
                    continue;

                uint8_t iBlockType = m_iBlocks[iIndex];
                if (iBlockType == AIR)
                    continue;

                // Greedy meshing / Face culling checks
                // We only draw a face if the neighbor is Air (0)

                // UP
                if (iY == CHUNK_HEIGHT - 1 || GetBlockAt(iX, iY + 1, iZ) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::UP, iBlockType);

                // DOWN
                if (iY == 0 || GetBlockAt(iX, iY - 1, iZ) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::DOWN, iBlockType);

                // RIGHT (X+)
                if (GetBlockAt(iX + 1, iY, iZ) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::RIGHT, iBlockType);

                // LEFT (X-)
                if (GetBlockAt(iX - 1, iY, iZ) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::LEFT, iBlockType);

                // FRONT (Z+)
                if (GetBlockAt(iX, iY, iZ + 1) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::FRONT, iBlockType);

                // BACK (Z-)
                if (GetBlockAt(iX, iY, iZ - 1) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::BACK, iBlockType);
            }
        }
    }
}

//*********************************************************************
void Chunk::addBlockFace(int iX, int iY, int iZ, FaceDirection iDir, int iBlockType) {
    unsigned int iStartIndex = static_cast<unsigned int>(m_vec_fVertices.size()) / 5;

    float fX = static_cast<float>(iX) + (CHUNK_SIZE * static_cast<float>(m_iChunkX));
    float fY = static_cast<float>(iY);
    float fZ = static_cast<float>(iZ) + (CHUNK_SIZE * static_cast<float>(m_iChunkZ));

    // Texture Atlas Calculations
    int iAtlasCol = 0, iAtlasRow = 0;

    // Simple Texture Mapper
    if (iBlockType == GRASS) {
        if (iDir == FaceDirection::UP) {
            iAtlasCol = 2;
            iAtlasRow = 9;
        }  // Grass Top
        else if (iDir == FaceDirection::DOWN) {
            iAtlasCol = 1;
            iAtlasRow = 0;
        }  // Stone
        else {
            iAtlasCol = 3;
            iAtlasRow = 0;
        }  // Grass Side
    } else if (iBlockType == DIRT) {
        iAtlasCol = 2;
        iAtlasRow = 0;  // Dirt
    } else if (iBlockType == STONE) {
        iAtlasCol = 1;
        iAtlasRow = 0;  // Stone
    }

    float fSlotSize = 1.0f / 16.0f;  // Assuming 16x16 Texture Atlas
    float u0 = static_cast<float>(iAtlasCol) * fSlotSize;
    float v1 = static_cast<float>(iAtlasRow) * fSlotSize;
    float u1 = u0 + fSlotSize;
    float v0 = v1 + fSlotSize;

    // Face Geometry
    // Format: X, Y, Z, U, V
    switch (iDir) {
        case FaceDirection::FRONT:  // Z+
            m_vec_fVertices.insert(
                m_vec_fVertices.end(),
                {
                    fX,     fY,     fZ + 1, u0, v0, fX + 1, fY,     fZ + 1, u1, v0,
                    fX + 1, fY + 1, fZ + 1, u1, v1, fX,     fY + 1, fZ + 1, u0, v1,
                });
            break;

        case FaceDirection::BACK:  // Z-
            m_vec_fVertices.insert(m_vec_fVertices.end(),
                                   {
                                       fX,     fY,     fZ, u0, v0, fX,     fY + 1, fZ, u0, v1,
                                       fX + 1, fY + 1, fZ, u1, v1, fX + 1, fY,     fZ, u1, v0,
                                   });
            break;

        case FaceDirection::RIGHT:  // X+
            m_vec_fVertices.insert(
                m_vec_fVertices.end(),
                {
                    fX + 1, fY,     fZ,     u0, v0, fX + 1, fY + 1, fZ,     u0, v1,
                    fX + 1, fY + 1, fZ + 1, u1, v1, fX + 1, fY,     fZ + 1, u1, v0,
                });
            break;

        case FaceDirection::LEFT:  // X-
            m_vec_fVertices.insert(m_vec_fVertices.end(),
                                   {
                                       fX, fY,     fZ,     u0, v0, fX, fY,     fZ + 1, u1, v0,
                                       fX, fY + 1, fZ + 1, u1, v1, fX, fY + 1, fZ,     u0, v1,
                                   });
            break;

        case FaceDirection::UP:  // Y+
            m_vec_fVertices.insert(
                m_vec_fVertices.end(),
                {
                    fX,     fY + 1, fZ,     u0, v1, fX,     fY + 1, fZ + 1, u0, v0,
                    fX + 1, fY + 1, fZ + 1, u1, v0, fX + 1, fY + 1, fZ,     u1, v1,
                });
            break;

        case FaceDirection::DOWN:  // Y-
            m_vec_fVertices.insert(m_vec_fVertices.end(),
                                   {
                                       fX,     fY, fZ,     u0, v1, fX + 1, fY, fZ,     u1, v1,
                                       fX + 1, fY, fZ + 1, u1, v0, fX,     fY, fZ + 1, u0, v0,
                                   });
            break;
    }

    // Indices (Quad -> 2 Triangles)
    m_vec_uiIndices.push_back(iStartIndex + 0);
    m_vec_uiIndices.push_back(iStartIndex + 1);
    m_vec_uiIndices.push_back(iStartIndex + 2);
    m_vec_uiIndices.push_back(iStartIndex + 2);
    m_vec_uiIndices.push_back(iStartIndex + 3);
    m_vec_uiIndices.push_back(iStartIndex + 0);
}

//*********************************************************************
void Chunk::UploadMesh() {
    if (!m_pVAO) {
        m_pVAO = new Renderer::VertexArray();
    }
    updateBuffers();

    // Clear CPU buffers after upload to save RAM
    m_vec_fVertices.clear();
    m_vec_uiIndices.clear();
}
//*********************************************************************
void Chunk::Render() const {
    if (m_pVAO && m_pVBO && m_pIBO) {
        m_pVAO->Bind();
        glDrawElements(GL_TRIANGLES, m_pIBO->GetCount(), GL_UNSIGNED_INT, 0);
        m_pVAO->Unbind();
    }
}
//*********************************************************************
void Chunk::ThermalStep(float fThermalDiffusivity, float fDeltaTime) {
    float fCoefficient = fThermalDiffusivity * fDeltaTime;

    for (int iZ = 0; iZ < CHUNK_SIZE; ++iZ) {
        for (int iY = 0; iY < CHUNK_HEIGHT; ++iY) {
            for (int iX = 0; iX < CHUNK_SIZE; ++iX) {
                int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);

                float fCurrentTemp = m_pfCurrFrameData[iIndex];
                float fNeighborSum =
                    GetTemperatureAt(iX + 1, iY, iZ) + GetTemperatureAt(iX - 1, iY, iZ) +
                    GetTemperatureAt(iX, iY + 1, iZ) + GetTemperatureAt(iX, iY - 1, iZ) +
                    GetTemperatureAt(iX, iY, iZ + 1) + GetTemperatureAt(iX, iY, iZ - 1);
                m_pfNextFrameData[iIndex] =
                    fCurrentTemp + fCoefficient * (fNeighborSum - 6.0f * fCurrentTemp);
            }
        }
    }

    SwapBuffers();
}
//*********************************************************************
void Chunk::DebugPrintThermalSlice() {
    int iY = CHUNK_HEIGHT / 2;
    std::cout << "--- Thermal Slice (Y=" << iY << ") ---\n";
    for (int iZ = 0; iZ < CHUNK_SIZE; ++iZ) {
        for (int iX = 0; iX < CHUNK_SIZE; ++iX) {
            float fTemp = m_pfCurrFrameData[GetFlatIndexOf3DLayer(iX, iY, iZ)];

            if (fTemp > 50.0f)
                std::cout << " # ";
            else if (fTemp > 10.0f)
                std::cout << " * ";
            else if (fTemp > 1.0f)
                std::cout << " . ";
            else
                std::cout << "   ";
        }
        std::cout << "\n";
    }
}