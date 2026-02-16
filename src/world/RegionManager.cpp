#include "RegionManager.h"
#include <iostream>

// ********************************************************************
RegionManager::RegionManager(const std::string& strWorldDir) : m_strWorldDir(strWorldDir) {
    if (!std::filesystem::exists(m_strWorldDir)) {
        std::filesystem::create_directory(m_strWorldDir);
    }
}
// ********************************************************************
RegionManager::~RegionManager() {
    std::lock_guard<std::mutex> lock(m_mutexIO);
    for (auto& pair : m_mapOpenFiles) {
        std::fstream* pFile = pair.second;
        if (pFile->is_open()) {
            pFile->close();
        }
        delete pFile;
    }
    m_mapOpenFiles.clear();
}
// ********************************************************************
std::string RegionManager::getRegionFileName(int iChunkX, int iChunkZ) const {
    auto [iRegionX, iRegionZ] = getRegionCoords(iChunkX, iChunkZ);
    return m_strWorldDir + "/r." + std::to_string(iRegionX) + "." + std::to_string(iRegionZ) +
           ".mcr";
}
// ********************************************************************
std::fstream* RegionManager::getRegionFile(int iChunkX, int iChunkZ) {
    std::string strFileName = getRegionFileName(iChunkX, iChunkZ);

    // 1. Check Cache
    auto itr = m_mapOpenFiles.find(strFileName);
    if (itr != m_mapOpenFiles.end()) {
        return itr->second;
    }

    // 2. Prepare new file stream
    std::fstream* pFile = new std::fstream();

    // Try opening existing file in Read/Write mode
    pFile->open(strFileName, std::ios::in | std::ios::out | std::ios::binary);

    // 3. If file doesn't exist, create it
    if (!pFile->is_open()) {
        // Open in Out mode to create the file
        pFile->open(strFileName, std::ios::out | std::ios::binary);

        if (!pFile->is_open()) {
            delete pFile;
            std::cerr << "[Error] Could not create region file: " << strFileName << std::endl;
            return nullptr;
        }

        // Initialize Header (Fill with 0s)
        // This reserves space for the lookup table
        std::vector<char> vecEmptyHeader(HEADER_SIZE, 0);
        pFile->write(vecEmptyHeader.data(), HEADER_SIZE);

        // Close and Re-open in Read/Write mode
        pFile->close();
        pFile->open(strFileName, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (!pFile->is_open()) {
        delete pFile;
        return nullptr;
    }

    m_mapOpenFiles[strFileName] = pFile;
    return pFile;
}
// ********************************************************************
bool RegionManager::SaveChunk(const Chunk& objChunk) {
    std::lock_guard<std::mutex> lock(m_mutexIO);

    std::fstream* pFile = getRegionFile(objChunk.GetChunkX(), objChunk.GetChunkZ());
    if (!pFile)
        return false;

    // Calculate header position
    int iLocalX = objChunk.GetChunkX() & REGION_MASK;
    int iLocalZ = objChunk.GetChunkZ() & REGION_MASK;
    int iHeaderOffset = (iLocalX + iLocalZ * REGION_WIDTH) * sizeof(int);

    // 1. Write Data to End of File (Append)
    // Note: A real Region format would try to fit this in a gap if a chunk was deleted,
    // but simply appending is fine for a basic engine.
    pFile->seekp(0, std::ios::end);
    int iChunkFileOffset = static_cast<int>(pFile->tellp());

    int iDataSize = CHUNK_VOL;  // 16*16*16 = 4096 bytes
    const uint8_t* pBlockData = objChunk.GetBlockData();

    pFile->write(reinterpret_cast<const char*>(&iDataSize), sizeof(int));
    pFile->write(reinterpret_cast<const char*>(pBlockData), iDataSize);

    // 2. Update Header with new offset
    pFile->seekp(iHeaderOffset, std::ios::beg);
    pFile->write(reinterpret_cast<const char*>(&iChunkFileOffset), sizeof(int));

    return true;
}
// ********************************************************************
bool RegionManager::LoadChunk(Chunk& objChunk) {
    std::lock_guard<std::mutex> lock(m_mutexIO);

    std::fstream* pFile = getRegionFile(objChunk.GetChunkX(), objChunk.GetChunkZ());
    if (!pFile)
        return false;

    // Calculate header position
    int iLocalX = objChunk.GetChunkX() & REGION_MASK;
    int iLocalZ = objChunk.GetChunkZ() & REGION_MASK;
    int iHeaderOffset = (iLocalX + iLocalZ * REGION_WIDTH) * sizeof(int);

    // 1. Read Offset from Header
    pFile->seekg(iHeaderOffset, std::ios::beg);
    int iChunkOffset = 0;
    pFile->read(reinterpret_cast<char*>(&iChunkOffset), sizeof(int));

    // If offset is 0, chunk has never been saved
    if (iChunkOffset == 0) {
        return false;
    }

    // 2. Read Chunk Data
    pFile->seekg(iChunkOffset, std::ios::beg);
    int iDataSize = 0;
    pFile->read(reinterpret_cast<char*>(&iDataSize), sizeof(int));

    // Basic Validation
    if (iDataSize != CHUNK_VOL) {
        std::cerr << "[Error] Corrupt chunk data at " << iChunkOffset << std::endl;
        return false;
    }

    std::vector<uint8_t> vecChunkData(iDataSize);
    pFile->read(reinterpret_cast<char*>(vecChunkData.data()), iDataSize);
    objChunk.SetBlockData(vecChunkData.data());

    return true;
}
// ********************************************************************