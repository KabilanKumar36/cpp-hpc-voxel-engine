/**
 * @file Chunk.cpp
 * @brief Implementation of Chunk meshing, physics colliders, and thermal diffusion integration.
 */

#include <cstdlib>

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
    size_t iRawBytes = PADDED_CHUNK_VOL * sizeof(float);
    size_t iAlignedBytes = (iRawBytes + 63) & ~63;
    m_pfCurrFrameData = static_cast<float*>(ALLOCATE_ALIGNED(iAlignedBytes, 64));
    m_pfNextFrameData = static_cast<float*>(ALLOCATE_ALIGNED(iAlignedBytes, 64));
    std::fill_n(m_pfCurrFrameData, PADDED_CHUNK_VOL, 0.0f);
    std::fill_n(m_pfNextFrameData, PADDED_CHUNK_VOL, 0.0f);
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

    for (int i = 0; i < 6; i++) {
        if (m_pNeighbours[i]) {
            Direction iOppDir = Direction::NORTH;
            if (i == Direction::NORTH)
                iOppDir = Direction::SOUTH;
            else if (i == Direction::EAST)
                iOppDir = Direction::WEST;
            else if (i == Direction::WEST)
                iOppDir = Direction::EAST;
            else if (i == Direction::ABOVE)
                iOppDir = Direction::BELOW;
            else if (i == Direction::BELOW)
                iOppDir = Direction::ABOVE;
            m_pNeighbours[i]->SetNeighbours(iOppDir, nullptr);
            m_pNeighbours[i] = nullptr;
        }
    }
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
      m_pThermalTex(std::move(other.m_pThermalTex)),
      m_iChunkX(other.m_iChunkX),
      m_iChunkZ(other.m_iChunkZ) {
    other.m_pVAO = nullptr;
    other.m_pVBO = nullptr;
    other.m_pIBO = nullptr;
    other.m_pfCurrFrameData = nullptr;
    other.m_pfNextFrameData = nullptr;

    std::memcpy(m_iBlocks, other.m_iBlocks, sizeof(m_iBlocks));
    std::memcpy(m_iHeightData, other.m_iHeightData, sizeof(m_iHeightData));

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
        m_pThermalTex = std::move(other.m_pThermalTex);
        m_iChunkX = other.m_iChunkX;
        m_iChunkZ = other.m_iChunkZ;

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

            int iHeight = static_cast<int>((fNoiseVal + 1.0f) * 10.0f);
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
        m_uiVertexCount = m_vec_fVertices.size() / 5;
        m_pVBO = new Renderer::VertexBuffer(
            m_vec_fVertices.data(),
            static_cast<unsigned int>(m_vec_fVertices.size()) * sizeof(float));
    }

    if (!m_vec_uiIndices.empty()) {
        m_uiTriangleCount = m_vec_uiIndices.size() / 3;
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
void Chunk::ReconstructMesh(bool bEnableNeighborCulling) {
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

                // UP (Y+)
                if (!bEnableNeighborCulling ||
                    (iY == CHUNK_HEIGHT - 1 || GetBlockAt(iX, iY + 1, iZ) == 0))
                    addBlockFace(iX, iY, iZ, FaceDirection::UP, iBlockType);

                // DOWN (Y-)
                if (!bEnableNeighborCulling || (iY == 0 || GetBlockAt(iX, iY - 1, iZ) == 0))
                    addBlockFace(iX, iY, iZ, FaceDirection::DOWN, iBlockType);

                // RIGHT (X+)
                if (!bEnableNeighborCulling || GetBlockAt(iX + 1, iY, iZ) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::RIGHT, iBlockType);

                // LEFT (X-)
                if (!bEnableNeighborCulling || GetBlockAt(iX - 1, iY, iZ) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::LEFT, iBlockType);

                // FRONT (Z+)
                if (!bEnableNeighborCulling || GetBlockAt(iX, iY, iZ + 1) == 0)
                    addBlockFace(iX, iY, iZ, FaceDirection::FRONT, iBlockType);

                // BACK (Z-)
                if (!bEnableNeighborCulling || GetBlockAt(iX, iY, iZ - 1) == 0)
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

    float fSlotSize = 1.0f / 16.0f;
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
float Chunk::GetTemperatureAt(int iX, int iY, int iZ) const {
    if (iX >= 0 && iX < CHUNK_SIZE && iY >= 0 && iY < CHUNK_HEIGHT && iZ >= 0 && iZ < CHUNK_SIZE) {
        return m_pfCurrFrameData[GetPaddedIndexOf3DLayer(iX, iY, iZ)];
    }
    // Boundary checks (Neighbor querying)
    if (iX < 0) {
        if (m_pNeighbours[Direction::WEST])
            return m_pNeighbours[Direction::WEST]->GetTemperatureAt(CHUNK_SIZE - 1, iY, iZ);
        else if (m_bVonNeumannBC)
            return m_pfCurrFrameData[GetPaddedIndexOf3DLayer(0, iY, iZ)];
    } else if (iX >= CHUNK_SIZE) {
        if (m_pNeighbours[Direction::EAST])
            return m_pNeighbours[Direction::EAST]->GetTemperatureAt(0, iY, iZ);
        else if (m_bVonNeumannBC)
            return m_pfCurrFrameData[GetPaddedIndexOf3DLayer(CHUNK_SIZE - 1, iY, iZ)];
    }

    if (iY < 0) {
        if (m_pNeighbours[Direction::BELOW])
            return m_pNeighbours[Direction::BELOW]->GetTemperatureAt(iX, CHUNK_HEIGHT - 1, iZ);
        else if (m_bVonNeumannBC)
            return m_pfCurrFrameData[GetPaddedIndexOf3DLayer(iX, 0, iZ)];
    } else if (iY >= CHUNK_HEIGHT) {
        if (m_pNeighbours[Direction::ABOVE])
            return m_pNeighbours[Direction::ABOVE]->GetTemperatureAt(iX, 0, iZ);
        else if (m_bVonNeumannBC)
            return m_pfCurrFrameData[GetPaddedIndexOf3DLayer(iX, CHUNK_SIZE - 1, iZ)];
    }

    if (iZ < 0) {
        if (m_pNeighbours[Direction::SOUTH])
            return m_pNeighbours[Direction::SOUTH]->GetTemperatureAt(iX, iY, CHUNK_SIZE - 1);
        else if (m_bVonNeumannBC)
            return m_pfCurrFrameData[GetPaddedIndexOf3DLayer(iX, iY, 0)];
    } else if (iZ >= CHUNK_SIZE) {
        if (m_pNeighbours[Direction::NORTH])
            return m_pNeighbours[Direction::NORTH]->GetTemperatureAt(iX, iY, 0);
        else if (m_bVonNeumannBC)
            return m_pfCurrFrameData[GetPaddedIndexOf3DLayer(iX, iY, CHUNK_SIZE - 1)];
    }

    return 0.0f;
}
//*********************************************************************
void Chunk::ThermalStep(float fThermalDiffusivity, float fDeltaTime) {
    float fCoefficient = fThermalDiffusivity * fDeltaTime;

    for (int iZ = 0; iZ < CHUNK_SIZE; ++iZ) {
        for (int iY = 0; iY < CHUNK_HEIGHT; ++iY) {
            for (int iX = 0; iX < CHUNK_SIZE; ++iX) {
                int iPaddedIndex = GetPaddedIndexOf3DLayer(iX, iY, iZ);
                float fCurrentTemp = m_pfCurrFrameData[iPaddedIndex];
                float fNeighborSum =
                    GetTemperatureAt(iX + 1, iY, iZ) + GetTemperatureAt(iX - 1, iY, iZ) +
                    GetTemperatureAt(iX, iY + 1, iZ) + GetTemperatureAt(iX, iY - 1, iZ) +
                    GetTemperatureAt(iX, iY, iZ + 1) + GetTemperatureAt(iX, iY, iZ - 1);
                m_pfNextFrameData[iPaddedIndex] =
                    fCurrentTemp + fCoefficient * (fNeighborSum - 6.0f * fCurrentTemp);
            }
        }
    }
}
//*********************************************************************
void Chunk::ThermalStep_AVX2(float fThermalDiffusivity, float fDeltaTime) {
    float fCoefficient = fThermalDiffusivity * fDeltaTime;
    const int iOffsetY = PADDED_CHUNK_SIZE;
    const int iOffsetZ = PADDED_CHUNK_SIZE * PADDED_CHUNK_HEIGHT;

    __m256 vecCoeff = _mm256_set1_ps(fCoefficient);
    __m256 vecSix = _mm256_set1_ps(6.0f);

    // Phase 1 : The Core (Pure AVX2 SIMD Speed)
    for (int iZ = 0; iZ < CHUNK_SIZE; ++iZ) {
        for (int iY = 0; iY < CHUNK_HEIGHT; ++iY) {
            int iX = 0;
            int iIndex = (iX + 1) + (iY + 1) * iOffsetY + (iZ + 1) * iOffsetZ;

            for (; iX < CHUNK_SIZE - 8; iX += 8, iIndex += 8) {
                __m256 curr = _mm256_loadu_ps(&m_pfCurrFrameData[iIndex]);

                __m256 x1 = _mm256_loadu_ps(&m_pfCurrFrameData[iIndex - 1]);
                __m256 x2 = _mm256_loadu_ps(&m_pfCurrFrameData[iIndex + 1]);
                __m256 y1 = _mm256_loadu_ps(&m_pfCurrFrameData[iIndex - iOffsetY]);
                __m256 y2 = _mm256_loadu_ps(&m_pfCurrFrameData[iIndex + iOffsetY]);
                __m256 z1 = _mm256_loadu_ps(&m_pfCurrFrameData[iIndex - iOffsetZ]);
                __m256 z2 = _mm256_loadu_ps(&m_pfCurrFrameData[iIndex + iOffsetZ]);

                __m256 neighborSum =
                    _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(x1, x2), _mm256_add_ps(y1, y2)),
                                  _mm256_add_ps(z1, z2));

                __m256 next = _mm256_fmadd_ps(
                    vecCoeff, _mm256_sub_ps(neighborSum, _mm256_mul_ps(vecSix, curr)), curr);

                _mm256_storeu_ps(&m_pfNextFrameData[iIndex], next);
            }

            for (; iX < CHUNK_SIZE; ++iX, ++iIndex) {
                float fCurrentTemp = m_pfCurrFrameData[iIndex];
                float fNeighborSum =
                    m_pfCurrFrameData[iIndex - 1] + m_pfCurrFrameData[iIndex + 1] +
                    m_pfCurrFrameData[iIndex - iOffsetY] + m_pfCurrFrameData[iIndex + iOffsetY] +
                    m_pfCurrFrameData[iIndex - iOffsetZ] + m_pfCurrFrameData[iIndex + iOffsetZ];
                m_pfNextFrameData[iIndex] =
                    fCurrentTemp + fCoefficient * (fNeighborSum - 6.0f * fCurrentTemp);
            }
        }
    }

    // Phase 2 : The Borders (Non-SIMD, Boundary Checks)
    for (int iZ = 0; iZ < CHUNK_SIZE; ++iZ) {
        for (int iY = 0; iY < CHUNK_HEIGHT; ++iY) {
            for (int iX = 0; iX < CHUNK_SIZE; ++iX) {
                if (iX > 0 && iX < CHUNK_SIZE - 1 && iY > 0 && iY < CHUNK_HEIGHT - 1 && iZ > 0 &&
                    iZ < CHUNK_SIZE - 1)
                    continue;
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
}
//*********************************************************************
void Chunk::UpdateThermalTexture() {
    for (int iZ = -1; iZ <= CHUNK_SIZE; ++iZ) {
        for (int iY = -1; iY <= CHUNK_HEIGHT; ++iY) {
            for (int iX = -1; iX <= CHUNK_SIZE; ++iX) {
                if (iX >= 0 && iX < CHUNK_SIZE && iY >= 0 && iY < CHUNK_HEIGHT && iZ >= 0 &&
                    iZ < CHUNK_SIZE)
                    continue;

                m_pfCurrFrameData[GetPaddedIndexOf3DLayer(iX, iY, iZ)] =
                    GetTemperatureAt(iX, iY, iZ);
            }
        }
    }
    if (!m_pThermalTex)
        m_pThermalTex = std::make_unique<Renderer::ThermalVolume>(
            PADDED_CHUNK_SIZE, PADDED_CHUNK_HEIGHT, PADDED_CHUNK_SIZE);
    if (m_pThermalTex && m_pfCurrFrameData) {
        m_pThermalTex->Update(m_pfCurrFrameData);
    }
}