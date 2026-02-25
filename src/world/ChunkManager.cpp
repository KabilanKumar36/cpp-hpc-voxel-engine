#include "ChunkManager.h"
#include <iostream>

//*********************************************************************
void ChunkManager::Update(float fPlayerX, float fPlayerZ) {
    // 1. Process Finished Chunks from ThreadPool
    std::optional<Chunk> optChunk;
    while ((optChunk = m_objFinishedQueue.try_pop()).has_value()) {
        auto pNewChunk = std::make_unique<Chunk>(std::move(optChunk.value()));
        int iCX = pNewChunk->GetChunkX();
        int iCZ = pNewChunk->GetChunkZ();

        // Move into main map
        m_mapChunks[std::make_pair(iCX, iCZ)] = std::move(pNewChunk);
        Chunk* pActiveChunk = m_mapChunks[{iCX, iCZ}].get();
        updateChunkNeighbours(pActiveChunk);

        // Generate mesh on Main Thread (OpenGL requires this)
        pActiveChunk->ReconstructMesh();
        pActiveChunk->UploadMesh();

        // Remove from pending set
        {
            std::lock_guard<std::mutex> lock(m_mutexPending);
            m_setPendingCoords.erase({iCX, iCZ});
        }
    }

    // 2. Determine Current Chunk
    int iCurrentChunkX = static_cast<int>(std::floor(fPlayerX / CHUNK_SIZE));
    int iCurrentChunkZ = static_cast<int>(std::floor(fPlayerZ / CHUNK_SIZE));

    // Optimization: Only update loading logic if player moved to a new chunk
    if (iCurrentChunkX == m_iLastPlayerChunkX && iCurrentChunkZ == m_iLastPlayerChunkZ) {
        return;
    }

    m_iLastPlayerChunkX = iCurrentChunkX;
    m_iLastPlayerChunkZ = iCurrentChunkZ;

    // 3. Unload Far Chunks
    for (auto itr = m_mapChunks.begin(); itr != m_mapChunks.end();) {
        int iChunkX = itr->first.first;
        int iChunkZ = itr->first.second;

        if (std::abs(iChunkX - iCurrentChunkX) > m_iRenderDistance + 2 ||
            std::abs(iChunkZ - iCurrentChunkZ) > m_iRenderDistance + 2) {
            // Save before unloading (Optional, adds lag spike)
            // m_objRegionManager.SaveChunk(*itr->second);
            itr = m_mapChunks.erase(itr);
        } else {
            ++itr;
        }
    }

    // 4. Queue New Chunks
    for (int iX = iCurrentChunkX - m_iRenderDistance; iX <= iCurrentChunkX + m_iRenderDistance;
         iX++) {
        for (int iZ = iCurrentChunkZ - m_iRenderDistance; iZ <= iCurrentChunkZ + m_iRenderDistance;
             iZ++) {
            std::pair<int, int> ChunkCoord = {iX, iZ};

            // Skip if already loaded
            if (m_mapChunks.count(ChunkCoord))
                continue;

            // Skip if already pending
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

//*********************************************************************
void ChunkManager::SetBlock(int iWorldX, int iWorldY, int iWorldZ, uint8_t iBlockType) {
    if (iWorldY < 0 || iWorldY >= CHUNK_HEIGHT)
        return;

    int iChunkX = static_cast<int>(std::floor(static_cast<float>(iWorldX) / CHUNK_SIZE));
    int iChunkZ = static_cast<int>(std::floor(static_cast<float>(iWorldZ) / CHUNK_SIZE));

    Chunk* pChunk = GetChunk(iChunkX, iChunkZ);
    if (!pChunk)
        return;

    // Convert to local coords
    int iLocalX = iWorldX % CHUNK_SIZE;
    int iLocalZ = iWorldZ % CHUNK_SIZE;
    if (iLocalX < 0)
        iLocalX += CHUNK_SIZE;
    if (iLocalZ < 0)
        iLocalZ += CHUNK_SIZE;

    pChunk->SetBlockAt(iLocalX, iWorldY, iLocalZ, iBlockType);
    pChunk->ReconstructMesh();
    pChunk->UploadMesh();

    // Logic: If placing a block, convert grass below to dirt
    if (iBlockType != 0 && iWorldY > 0) {
        uint8_t iBelowBlock = pChunk->GetBlockAt(iLocalX, iWorldY - 1, iLocalZ);
        if (iBelowBlock == GRASS) {
            pChunk->SetBlockAt(iLocalX, iWorldY - 1, iLocalZ, DIRT);
        }
    }

    // Update Neighbors if on boundary
    if (iLocalX == 0) {
        if (Chunk* pWest = GetChunk(iChunkX - 1, iChunkZ)) {
            pWest->ReconstructMesh();
            pWest->UploadMesh();
        }
    }
    if (iLocalX == CHUNK_SIZE - 1) {
        if (Chunk* pEast = GetChunk(iChunkX + 1, iChunkZ)) {
            pEast->ReconstructMesh();
            pEast->UploadMesh();
        }
    }
    if (iLocalZ == 0) {
        if (Chunk* pSouth = GetChunk(iChunkX, iChunkZ - 1)) {
            pSouth->ReconstructMesh();
            pSouth->UploadMesh();
        }
    }
    if (iLocalZ == CHUNK_SIZE - 1) {
        if (Chunk* pNorth = GetChunk(iChunkX, iChunkZ + 1)) {
            pNorth->ReconstructMesh();
            pNorth->UploadMesh();
        }
    }
}

//*********************************************************************
void ChunkManager::SaveWorld() {
    std::cout << "Saving world..." << std::endl;
    for (auto& pair : m_mapChunks) {
        m_objRegionManager.SaveChunk(*pair.second);
    }
}

//*********************************************************************
Chunk* ChunkManager::GetChunk(int iX, int iZ) {
    auto itr = m_mapChunks.find({iX, iZ});
    if (itr != m_mapChunks.end())
        return itr->second.get();
    return nullptr;
}

//*********************************************************************
const Chunk* ChunkManager::GetChunk(int iX, int iZ) const {
    auto itr = m_mapChunks.find({iX, iZ});
    if (itr != m_mapChunks.end())
        return itr->second.get();
    return nullptr;
}

//*********************************************************************
void ChunkManager::enqueueLoadChunk(int iX, int iZ) {
    {
        std::lock_guard<std::mutex> lock(m_mutexPending);
        m_setPendingCoords.insert({iX, iZ});
    }

    // Async Job
    m_objThreadPool.submit([this, iX, iZ]() {
        Chunk objChunk(iX, iZ);
        m_objRegionManager.LoadChunk(objChunk);
        m_objFinishedQueue.push(std::move(objChunk));
        // 1. Create the chunk on the heap via unique_ptr
        /*auto pChunk = std::make_unique<Chunk>(iX, iZ);

        // 2. Load data
        m_objRegionManager.LoadChunk(*pChunk);

        // 3. Move the WHOLE OBJECT into the queue
        // Update your FinishedQueue to take std::unique_ptr<Chunk> if possible,
        // OR move the value out of the pointer:
        m_objFinishedQueue.push(std::move(*pChunk)); */
    });
}

//*********************************************************************
void ChunkManager::updateChunkNeighbours(Chunk* pChunk) {
    int iX = pChunk->GetChunkX();
    int iZ = pChunk->GetChunkZ();

    auto Link = [&](Direction iDir, int iNX, int iNZ) {
        Chunk* pNeighbor = GetChunk(iNX, iNZ);
        if (pNeighbor) {
            pChunk->SetNeighbours(iDir, pNeighbor);

            Direction iOppDir;
            if (iDir == NORTH)
                iOppDir = SOUTH;
            else if (iDir == SOUTH)
                iOppDir = NORTH;
            else if (iDir == EAST)
                iOppDir = WEST;
            else
                iOppDir = EAST;

            pNeighbor->SetNeighbours(iOppDir, pChunk);

            // Trigger neighbor update if needed (optional optimization)
            // pNeighbor->ReconstructMesh();
            // pNeighbor->UploadMesh();
        }
    };

    Link(NORTH, iX, iZ + 1);
    Link(SOUTH, iX, iZ - 1);
    Link(EAST, iX + 1, iZ);
    Link(WEST, iX - 1, iZ);
}