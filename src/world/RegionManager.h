#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include "Chunk.h"

#define REGION_WIDTH 32
#define REGION_SIZE 5
#define REGION_MASK 31
#define REGION_AREA (REGION_WIDTH * REGION_WIDTH)
#define HEADER_SIZE (REGION_AREA * 4)

class RegionManager {
public:
    RegionManager(const std::string& strWorldDir);
    ~RegionManager();

    bool SaveChunk(const Chunk& objChunk);
    bool LoadChunk(Chunk& objChunk);

private:
    std::string m_strWorldDict;
    std::unordered_map<std::string, std::fstream*> m_mapOpenFiles;

    std::string GetRegionFileName(int iChunkX, int iChunkZ) const;
    std::fstream* GetRegionFile(int iChunkX, int iChunkZ);
    std::mutex m_mutexIO;

    std::pair<int, int> GetRegionCoords(int iChunkX, int iChunkZ) const {
        return {iChunkX >> REGION_SIZE, iChunkZ >> REGION_SIZE};
    }
};