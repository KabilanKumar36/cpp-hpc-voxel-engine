#pragma once
#include <cstdlib>
#include <vector>
#include "../core/FastNoiseLite.h"
#include "../renderer/Buffer.h"
#include "../renderer/IndexBuffer.h"
#include "../renderer/VertexArray.h"

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 16;
constexpr int CHUNK_VOL = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
enum Direction {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    UP,
    DOWN,

};
class Chunk {
public:
    Chunk() = delete;
    Chunk(int iX, int iZ) : m_iChunkX(iX), m_iChunkZ(iZ) {
        m_bEnableFaceCulling = false;
        updateHeightData();
    }
    ~Chunk();
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;

    Chunk(Chunk&& other) noexcept;
    Chunk& operator=(Chunk&& other) noexcept;

    [[nodiscard]] int GetChunkX() const { return m_iChunkX; }
    [[nodiscard]] int GetChunkZ() const { return m_iChunkZ; }
    [[nodiscard]] bool GetFaceCulling() const { return m_bEnableFaceCulling; }

    void Render() const;
    void SetFaceCulling(bool bOpt) { m_bEnableFaceCulling = bOpt; }
    [[nodiscard]] inline int GetFlatIndexOf3DLayer(int iX, int iY, int iZ) const {
        if (iX < 0 || iX >= CHUNK_SIZE || iY < 0 || iY >= CHUNK_HEIGHT || iZ < 0 ||
            iZ >= CHUNK_SIZE) {
            return -1;
        }

        return iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_HEIGHT);
    }
    uint8_t GetBlockAt(int iX, int iY, int iZ) const {
        int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
        if (iIndex == -1)
            return 0;
        return m_iBlocks[iIndex];
    }
    void SetBlockAt(int iX, int iY, int iZ, uint8_t uiBlockType) {
        int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
        if (iIndex == -1)
            return;
        m_iBlocks[iIndex] = uiBlockType;
    }
    void ReconstructMesh();
    void UploadMesh();

private:
    std::vector<float> m_vec_fVertices;
    std::vector<unsigned int> m_vec_uiIndices;

    Renderer::VertexArray* m_pVAO = nullptr;
    Renderer::VertexBuffer* m_pVBO = nullptr;
    Renderer::IndexBuffer* m_pIBO = nullptr;

    FastNoiseLite noise{};
    int m_iHeightData[CHUNK_SIZE][CHUNK_SIZE];
    int m_iChunkX = 0, m_iChunkZ = 0;
    uint8_t m_iBlocks[CHUNK_VOL]{0};

    bool m_bEnableFaceCulling = false;
    void updateHeightData();
    void updateBuffers();
    void addBlockFace(int iX, int iY, int iZ, Direction iDir, int iBlockType);
};