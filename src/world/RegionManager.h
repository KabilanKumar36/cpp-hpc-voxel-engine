#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "Chunk.h"

// Region File Format Constants
// 32x32 Chunks per Region File
#define REGION_WIDTH 32
#define REGION_SIZE 5   // 2^5 = 32
#define REGION_MASK 31  // 0x1F
#define REGION_AREA (REGION_WIDTH * REGION_WIDTH)
#define HEADER_SIZE (REGION_AREA * 4)  // 4 Bytes (int) per chunk offset

class RegionManager {
public:
    RegionManager(const std::string& strWorldDir);
    ~RegionManager();

    bool SaveChunk(const Chunk& objChunk);
    bool LoadChunk(Chunk& objChunk);

private:
    std::string m_strWorldDir;

    // Cache open file handles to avoid opening/closing files constantly
    std::unordered_map<std::string, std::fstream*> m_mapOpenFiles;
    std::mutex m_mutexIO;

    std::string getRegionFileName(int iChunkX, int iChunkZ) const;

    // Gets or creates a file stream for the specific region
    std::fstream* getRegionFile(int iChunkX, int iChunkZ);

    std::pair<int, int> getRegionCoords(int iChunkX, int iChunkZ) const {
        return {iChunkX >> REGION_SIZE, iChunkZ >> REGION_SIZE};
    }
};