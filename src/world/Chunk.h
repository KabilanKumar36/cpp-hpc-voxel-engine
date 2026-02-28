#pragma once

#include <FastNoiseLite.h>  // Ensure this library is in your include path
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "../core/MathUtils.h"
#include "../physics/AABB.h"
#include "../renderer/Buffer.h"
#include "../renderer/IndexBuffer.h"
#include "../renderer/ThermalVolume.h"
#include "../renderer/VertexArray.h"

// Constants
constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 16;
constexpr int CHUNK_VOL = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

// Enums
enum FaceDirection { FRONT, BACK, LEFT, RIGHT, UP, DOWN };
enum Direction { NORTH = 0, SOUTH, EAST, WEST, ABOVE, BELOW };  // Z+, Z-, X+, X-, Y+, Y-
enum BlockType { AIR = 0, GRASS = 1, DIRT = 2, STONE = 3 };

class Chunk {
public:
    Chunk() = delete;
    Chunk(int iX, int iZ);
    ~Chunk();

    // Delete Copy (Chunks are heavy)
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    // Move Semantics
    Chunk(Chunk&& other) noexcept;
    Chunk& operator=(Chunk&& other) noexcept;

    // --- Getters ---
    [[nodiscard]] int GetChunkX() const { return m_iChunkX; }
    [[nodiscard]] int GetChunkZ() const { return m_iChunkZ; }
    [[nodiscard]] bool GetFaceCulling() const { return m_bEnableFaceCulling; }

    [[nodiscard]] bool IsValid() const { return m_pVAO != nullptr; }
    const uint8_t* GetBlockData() const { return m_iBlocks; }

    // --- Core Logic ---

    /**
     * @brief Generates the AABB for the entire chunk based on max height.
     */
    AABB GetAABB() const;

    void Render() const;
    void SetFaceCulling(bool bOpt) { m_bEnableFaceCulling = bOpt; }

    void SetNeighbours(Direction iDir, Chunk* pChunk) { m_pNeighbours[iDir] = pChunk; }

    // --- Block Manipulation ---

    [[nodiscard]] inline int GetFlatIndexOf3DLayer(int iX, int iY, int iZ) const {
        if (iX < 0 || iX >= CHUNK_SIZE || iY < 0 || iY >= CHUNK_HEIGHT || iZ < 0 ||
            iZ >= CHUNK_SIZE) {
            return -1;
        }

        return iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_HEIGHT);
    }

    /**
     * @brief Gets a block ID. Handles boundary checks by querying neighbors.
     */
    uint8_t GetBlockAt(int iX, int iY, int iZ) const {
        // Quick check for internal blocks
        if (iX >= 0 && iX < CHUNK_SIZE && iY >= 0 && iY < CHUNK_HEIGHT && iZ >= 0 &&
            iZ < CHUNK_SIZE) {
            return m_iBlocks[iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_HEIGHT)];
        }

        // Boundary checks (Neighbor querying)
        if (iX < 0)
            return m_pNeighbours[Direction::WEST]
                       ? m_pNeighbours[Direction::WEST]->GetBlockAt(CHUNK_SIZE - 1, iY, iZ)
                       : 0;
        if (iX >= CHUNK_SIZE)
            return m_pNeighbours[Direction::EAST]
                       ? m_pNeighbours[Direction::EAST]->GetBlockAt(0, iY, iZ)
                       : 0;

        if (iY < 0)
            return m_pNeighbours[Direction::BELOW]
                       ? m_pNeighbours[Direction::BELOW]->GetBlockAt(iX, CHUNK_SIZE - 1, iZ)
                       : 0;
        if (iY >= CHUNK_HEIGHT)
            return m_pNeighbours[Direction::ABOVE]
                       ? m_pNeighbours[Direction::ABOVE]->GetBlockAt(iX, 0, iZ)
                       : 0;

        if (iZ < 0)
            return m_pNeighbours[Direction::SOUTH]
                       ? m_pNeighbours[Direction::SOUTH]->GetBlockAt(iX, iY, CHUNK_SIZE - 1)
                       : 0;
        if (iZ >= CHUNK_SIZE)
            return m_pNeighbours[Direction::NORTH]
                       ? m_pNeighbours[Direction::NORTH]->GetBlockAt(iX, iY, 0)
                       : 0;

        return 0;  // Default to Air
    }

    void SetBlockAt(int iX, int iY, int iZ, uint8_t uiBlockType);

    void SetBlockData(const uint8_t* iBlocks) {
        if (iBlocks)
            std::memcpy(m_iBlocks, iBlocks, CHUNK_VOL * sizeof(uint8_t));
    }

    inline float GetTemperatureAt(int iX, int iY, int iZ) const {
        if (iX >= 0 && iX < CHUNK_SIZE && iY >= 0 && iY < CHUNK_HEIGHT && iZ >= 0 &&
            iZ < CHUNK_SIZE) {
            return m_pfCurrFrameData[GetFlatIndexOf3DLayer(iX, iY, iZ)];
        }
        // Boundary checks (Neighbor querying)
        if (iX < 0 && m_pNeighbours[Direction::WEST])
            return m_pNeighbours[Direction::WEST]->GetTemperatureAt(CHUNK_SIZE - 1, iY, iZ);
        else if (iX >= CHUNK_SIZE && m_pNeighbours[Direction::EAST])
            return m_pNeighbours[Direction::EAST]->GetTemperatureAt(0, iY, iZ);

        if (iY < 0 && m_pNeighbours[Direction::BELOW])
            return m_pNeighbours[Direction::BELOW]->GetTemperatureAt(iX, CHUNK_HEIGHT - 1, iZ);
        else if (iY >= CHUNK_HEIGHT && m_pNeighbours[Direction::ABOVE])
            return m_pNeighbours[Direction::ABOVE]->GetTemperatureAt(iX, 0, iZ);

        if (iZ < 0 && m_pNeighbours[Direction::SOUTH])
            return m_pNeighbours[Direction::SOUTH]->GetTemperatureAt(iX, iY, CHUNK_SIZE - 1);
        else if (iZ >= CHUNK_SIZE && m_pNeighbours[Direction::NORTH])
            return m_pNeighbours[Direction::NORTH]->GetTemperatureAt(iX, iY, 0);

        return 0.0f;
    }
    // --- Generation ---
    void ReconstructMesh();
    void UploadMesh();
    void SwapBuffers() { std::swap(m_pfCurrFrameData, m_pfNextFrameData); }
    void InjectHeat(int iX, int iY, int iZ, float fTemp) {
        int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
        if (iIndex != -1) {
            m_pfCurrFrameData[iIndex] = fTemp;
        }
    }
    void ThermalStep(float fThermalDiffusivity, float fDeltaTime);
    void DebugPrintThermalSlice();
    float* GetCurrData() const { return m_pfCurrFrameData; }
    void UpdateThermalTexture();
    void Bind(int iVal) {
        if (m_pThermalTex)
            m_pThermalTex->Bind(iVal);
    }

private:
    std::vector<float> m_vec_fVertices;
    std::vector<unsigned int> m_vec_uiIndices;

    Renderer::VertexArray* m_pVAO = nullptr;
    Renderer::VertexBuffer* m_pVBO = nullptr;
    Renderer::IndexBuffer* m_pIBO = nullptr;

    Chunk* m_pNeighbours[6] = {nullptr};

    float* m_pfCurrFrameData = nullptr;
    float* m_pfNextFrameData = nullptr;
    std::unique_ptr<Renderer::ThermalVolume> m_pThermalTex;

    // Per-chunk noise instance (Consider moving to a global generator for efficiency)
    FastNoiseLite noise{};
    int m_iHeightData[CHUNK_SIZE][CHUNK_SIZE];
    int m_iChunkX = 0, m_iChunkZ = 0;
    uint8_t m_iBlocks[CHUNK_VOL]{0};

    bool m_bEnableFaceCulling = false;

    // Helpers
    void updateHeightData();
    void updateBuffers();
    void addBlockFace(int iX, int iY, int iZ, FaceDirection iDir, int iBlockType);
};