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

// Forward Declaration
namespace Renderer {
class Shader;
}

class ChunkManager {
public:
    ChunkManager() = delete;
    ChunkManager(std::string& strFolderPath) : m_objRegionManager(strFolderPath) {}

    /**
     * @brief Main update loop. Handles chunk loading/unloading based on player position.
     * @param fPlayerX Player world X.
     * @param fPlayerZ Player world Z.
     */
    void Update(float fPlayerX, float fPlayerZ);

    /**
     * @brief Modifies a block in the world. Handles mesh updates for the chunk and neighbors.
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

    // Helper for Renderer
    std::map<std::pair<int, int>, std::unique_ptr<Chunk>>& GetMutableChunks() {
        return m_mapChunks;
    }

private:
    void enqueueLoadChunk(int iX, int iZ);
    void updateChunkNeighbours(Chunk* pChunk);

    std::map<std::pair<int, int>, std::unique_ptr<Chunk>> m_mapChunks;

    // Threading Synchronization
    std::set<std::pair<int, int>> m_setPendingCoords;
    std::mutex m_mutexPending;

    RegionManager m_objRegionManager;

    // ThreadPool must be destroyed BEFORE the queue to avoid use-after-free
    Core::ThreadSafeQueue<Chunk> m_objFinishedQueue;
    Core::ThreadPool m_objThreadPool;

    int m_iRenderDistance = 6;
    int m_iLastPlayerChunkX = -999999;
    int m_iLastPlayerChunkZ = -999999;
};