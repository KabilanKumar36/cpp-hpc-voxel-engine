#pragma once

#include <vector>
#include <optional>
#include "Chunk.h"
#include "../core/ThreadPool.h"

class ChunkManager {
public:
    ChunkManager() {

    }

    void LoadChunk(int iX, int iZ) { 
        m_objThreadPool.submit([this, iX, iZ]() { 
            Chunk objChunk(iX, iZ);
            objChunk.ReconstructMesh();
            m_objFinishedQueue.push(std::move(objChunk));
            });
    }

    void Update() { 
        std::optional<Chunk> optChunk = m_objFinishedQueue.try_pop();
        while (optChunk.has_value()) {
            Chunk& objChunk = optChunk.value();
            objChunk.UploadMesh();
            m_vecChunks.push_back(std::move(objChunk));
            optChunk = m_objFinishedQueue.try_pop();
        }
    }

    const std::vector<Chunk>& GetChunks() { return m_vecChunks; }
    std::vector<Chunk>& GetMutableChunks() { return m_vecChunks; }

private:
    std::vector<Chunk> m_vecChunks;
    Core::ThreadSafeQueue<Chunk> m_objFinishedQueue; // Destroyed Last
    Core::ThreadPool m_objThreadPool; // Destroyed first

};