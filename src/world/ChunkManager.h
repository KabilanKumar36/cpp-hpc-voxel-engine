#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include "../core/ThreadPool.h"
#include "../core/ThreadSafeQueue.h"
#include "Chunk.h"

class ChunkManager {
public:
    ChunkManager() = default;

    void Update(float fPlayerX, float fPlayerZ) {
        std::optional<Chunk> optChunk;
        while ((optChunk = m_objFinishedQueue.try_pop()).has_value()) {
            Chunk& objChunk = optChunk.value();
            objChunk.UploadMesh();
            {
                std::lock_guard<std::mutex> lock(m_mutexPending);
                m_setPendingCoords.erase({objChunk.GetChunkX(), objChunk.GetChunkZ()});
            }

            m_mapChunks.insert(std::make_pair(
                std::make_pair(objChunk.GetChunkX(), objChunk.GetChunkZ()), std::move(objChunk)));
        }

        int iCurrentChunkX = static_cast<int>(std::floor(fPlayerX / CHUNK_SIZE));
        int iCurrentChunkZ = static_cast<int>(std::floor(fPlayerZ / CHUNK_SIZE));
        if (iCurrentChunkX == m_iLastPlayerChunkX && iCurrentChunkZ == m_iLastPlayerChunkZ) {
            return;
        }

        m_iLastPlayerChunkX = iCurrentChunkX;
        m_iLastPlayerChunkZ = iCurrentChunkZ;
        for (auto itr = m_mapChunks.begin(); itr != m_mapChunks.end();) {
            int iChunkX = itr->first.first;
            int iChunkZ = itr->first.second;
            if (std::abs(iChunkX - iCurrentChunkX) > m_iRenderDistance + 2 ||
                std::abs(iChunkZ - iCurrentChunkZ) > m_iRenderDistance + 2) {
                itr = m_mapChunks.erase(itr);
            } else {
                ++itr;
            }
        }

        for (int iX = iCurrentChunkX - m_iRenderDistance; iX <= iCurrentChunkX + m_iRenderDistance;
             iX++) {
            for (int iZ = iCurrentChunkZ - m_iRenderDistance;
                 iZ <= iCurrentChunkZ + m_iRenderDistance;
                 iZ++) {
                std::pair<int, int> ChunkCoord = {iX, iZ};
                if (m_mapChunks.count(ChunkCoord))
                    continue;

                bool bPending = false;
                {
                    std::lock_guard<std::mutex> lock(m_mutexPending);
                    if (m_setPendingCoords.count(ChunkCoord))
                        bPending = true;
                }
                if (bPending)
                    continue;
                enqueueLoadChunk(iX, iZ);
            }
        }
    }

    Chunk* GetChunk(int iX, int iZ) {
        auto itr = m_mapChunks.find({iX, iZ});
        if (itr != m_mapChunks.end()) {
            return &itr->second;
        }
        return nullptr;
    }

    const Chunk* GetChunk(int iX, int iZ) const {
        auto itr = m_mapChunks.find({iX, iZ});
        if (itr != m_mapChunks.end()) {
            return &itr->second;
        }
        return nullptr;
    }
    std::map<std::pair<int, int>, Chunk>& GetMutableChunks() { return m_mapChunks; }

private:
    void enqueueLoadChunk(int iX, int iZ) {
        {
            std::lock_guard<std::mutex> lock(m_mutexPending);
            m_setPendingCoords.insert({iX, iZ});
        }
        m_objThreadPool.submit([this, iX, iZ]() {
            Chunk objChunk(iX, iZ);
            objChunk.ReconstructMesh();
            m_objFinishedQueue.push(std::move(objChunk));
        });
    }
    std::map<std::pair<int, int>, Chunk> m_mapChunks;
    std::set<std::pair<int, int>> m_setPendingCoords;
    std::mutex m_mutexPending;

    Core::ThreadSafeQueue<Chunk> m_objFinishedQueue;  // Destroyed Last
    Core::ThreadPool m_objThreadPool;                 // Destroyed first

    int m_iRenderDistance = 6;
    int m_iLastPlayerChunkX = -999999;
    int m_iLastPlayerChunkZ = -999999;
};