/**
 * @file ChunkManager.h
 * @brief Defines the ChunkManager class for managing the lifecycle, loading, and unloading of world
 * chunks.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>

#include "../core/ThreadPool.h"
#include "../core/ThreadSafeQueue.h"
#include "Chunk.h"
#include "RegionManager.h"

namespace Renderer {
class Shader;
}

/**
 * @class ChunkManager
 * @brief Orchestrates infinite world generation, active chunk tracking, and multi-threaded data
 * loading.
 */
class ChunkManager {
public:
    ChunkManager() = delete;
    ChunkManager(std::string& strFolderPath) : m_objRegionManager(strFolderPath) {}

    /**
     * @brief Core lifecycle loop. Synchronizes asynchronous chunks and manages the active render
     * radius.
     * @param fPlayerX Player world X.
     * @param fPlayerZ Player world Z.
     */
    void Update(float fPlayerX, float fPlayerZ);

    void ReloadAllChunks();

    /**
     * @brief Modifies a voxel block and invalidates necessary chunk meshes (including boundaries).
     */
    void SetBlock(int iWorldX, int iWorldY, int iWorldZ, uint8_t iBlockType);

    /**
     * @brief Persists all active chunks to disk.
     */
    void SaveWorld();

    /**
     * @brief Retrieves a pointer to a loaded chunk. Returns nullptr if not loaded.
     */
    Chunk* GetChunk(int iX, int iZ);
    const Chunk* GetChunk(int iX, int iZ) const;

    std::map<std::pair<int, int>, std::unique_ptr<Chunk>>& GetMutableChunks() {
        return m_mapChunks;
    }

    const std::map<std::pair<int, int>, std::unique_ptr<Chunk>>& GetChunks() const {
        return m_mapChunks;
    }
    void SetActiveThreads(int iCt) { m_iActiveThreads = iCt; }
    int GetActiveThreads() { return m_iActiveThreads; }

    size_t GetGeneratedVertCount() const { return m_iGeneratedVertexCount; }
    size_t GetGeneratedTriaCount() const { return m_iGeneratedTriangleCount; }

    void AddToUploadedVertCount(size_t iCt) { m_iUploadedVertexCount += iCt; }
    void ResetUploadedVertCount() { m_iUploadedVertexCount = 0; }
    size_t GetUploadedVertCount() const { return m_iUploadedVertexCount; }

    void AddToUploadedTriaCount(size_t iCt) { m_iUploadedTriangleCount += iCt; }
    void ResetUploadedTriaCount() { m_iUploadedTriangleCount = 0; }
    size_t GetUploadedTriaCount() const { return m_iUploadedTriangleCount; }

    void SetNeighborCulling(bool bNeighborCulling) { m_bEnableNeighborCulling = bNeighborCulling; }
    bool GetNeighborCulling() const { return m_bEnableNeighborCulling; }

private:
    void enqueueLoadChunk(int iX, int iZ);
    void updateChunkNeighbours(Chunk* pChunk);
    void updateGeneratedMeshStats();

    std::map<std::pair<int, int>, std::unique_ptr<Chunk>> m_mapChunks;

    std::set<std::pair<int, int>> m_setPendingCoords;
    std::mutex m_mutexPending;

    RegionManager m_objRegionManager;

    // ThreadPool must be destroyed BEFORE the queue to avoid use-after-free
    Core::ThreadSafeQueue<Chunk> m_objFinishedQueue;
    Core::ThreadPool m_objThreadPool;

    int m_iRenderDistance = 6;
    int m_iLastPlayerChunkX = -999999;
    int m_iLastPlayerChunkZ = -999999;
    int m_iActiveThreads = 4;
    size_t m_iGeneratedVertexCount = 0;
    size_t m_iGeneratedTriangleCount = 0;
    size_t m_iUploadedVertexCount = 0;
    size_t m_iUploadedTriangleCount = 0;
    bool m_bEnableNeighborCulling = true;
};