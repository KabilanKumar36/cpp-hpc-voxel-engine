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

            auto inserted = m_mapChunks.insert(std::make_pair(
                std::make_pair(objChunk.GetChunkX(), objChunk.GetChunkZ()), std::move(objChunk)));

            Chunk* pNewChunk = &inserted.first->second;
            updateChunkNeighbours(pNewChunk);
            pNewChunk->ReconstructMesh();
            pNewChunk->UploadMesh();

            {
                std::lock_guard<std::mutex> lock(m_mutexPending);
                m_setPendingCoords.erase({objChunk.GetChunkX(), objChunk.GetChunkZ()});
            }
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
    void SetBlock(int iWorldX, int iWorldY, int iWorldZ, uint8_t iBlockType) {
        if (iWorldY < 0 || iWorldY >= CHUNK_HEIGHT) {
            std::cout << "Cannot place block above: Height limit reached." << iWorldY << std::endl;
            return;
        }

        int iChunkX = static_cast<int>(std::floor(static_cast<float>(iWorldX) / CHUNK_SIZE));
        int iChunkZ = static_cast<int>(std::floor(static_cast<float>(iWorldZ) / CHUNK_SIZE));

        Chunk* pChunk = GetChunk(iChunkX, iChunkZ);
        if (!pChunk)
            return;

        int iLocalX = iWorldX % CHUNK_SIZE;
        int iLocalY = iWorldY;
        int iLocalZ = iWorldZ % CHUNK_SIZE;

        if (iLocalX < 0)
            iLocalX += CHUNK_SIZE;

        if (iLocalZ < 0)
            iLocalZ += CHUNK_SIZE;
        pChunk->SetBlockAt(iLocalX, iLocalY, iLocalZ, iBlockType);
        pChunk->ReconstructMesh();
        pChunk->UploadMesh();
        if (iBlockType != 0) {
            int iBelowY = iLocalY - 1;
            uint8_t iBelowBlockType = pChunk->GetBlockAt(iLocalX, iBelowY, iLocalZ);
            if (iBelowBlockType == 1) {
                pChunk->SetBlockAt(iLocalX, iBelowY, iLocalZ, 2);
            }
        }
        if (iLocalX == 0) {
            Chunk* pWest = GetChunk(iChunkX - 1, iChunkZ);
            if (pWest) {
                pWest->ReconstructMesh();
                pWest->UploadMesh();
            }
        }

        if (iLocalX == 15) {
            Chunk* pEast = GetChunk(iChunkX + 1, iChunkZ);
            if (pEast) {
                pEast->ReconstructMesh();
                pEast->UploadMesh();
            }
        }

        if (iLocalZ == 0) {
            Chunk* pSouth = GetChunk(iChunkX, iChunkZ - 1);
            if (pSouth) {
                pSouth->ReconstructMesh();
                pSouth->UploadMesh();
            }
        }

        if (iLocalZ == 15) {
            Chunk* pNorth = GetChunk(iChunkX, iChunkZ + 1);
            if (pNorth) {
                pNorth->ReconstructMesh();
                pNorth->UploadMesh();
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
            // objChunk.ReconstructMesh();
            m_objFinishedQueue.push(std::move(objChunk));
        });
    }
    void updateChunkNeighbours(Chunk* pChunk) {
        int iX = pChunk->GetChunkX();
        int iZ = pChunk->GetChunkZ();
        auto Link = [&](Direction iDir, int iNX, int iNZ) {
            Chunk* pNeighbor = GetChunk(iNX, iNZ);
            if (pNeighbor) {
                pChunk->SetNeighbours(iDir, pNeighbor);
                Direction iOppDir;
                if (iDir == Direction::NORTH)
                    iOppDir = SOUTH;
                else if (iDir == Direction::SOUTH)
                    iOppDir = NORTH;
                else if (iDir == Direction::EAST)
                    iOppDir = WEST;
                else /* if (iDir == Direction::WEST)*/
                    iOppDir = EAST;
                pNeighbor->SetNeighbours(iOppDir, pChunk);

                // pNeighbor->ReconstructMesh();
                // pNeighbor->UploadMesh();
            }
        };
        Link(Direction::NORTH, iX, iZ + 1);
        Link(Direction::SOUTH, iX, iZ - 1);
        Link(Direction::EAST, iX + 1, iZ);
        Link(Direction::WEST, iX - 1, iZ);
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