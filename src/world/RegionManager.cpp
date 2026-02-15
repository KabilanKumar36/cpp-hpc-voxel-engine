#include "RegionManager.h"
#include <iostream>
#include <filesystem>
#include <vector>

// ********************************************************************
RegionManager::RegionManager(const std::string& strWorldDict) : m_strWorldDict(strWorldDict) {
    if (!std::filesystem::exists(m_strWorldDict)) {
        std::filesystem::create_directory(m_strWorldDict);
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
std::string RegionManager::GetRegionFileName(int iChunkX, int iChunkZ) const {
    auto [iRegionX, iRegionZ] = GetRegionCoords(iChunkX, iChunkZ);
    return m_strWorldDict + "/r." + std::to_string(iRegionX) + "." + std::to_string(iRegionZ) + ".mcr";
}
// ********************************************************************
std::fstream* RegionManager::GetRegionFile(int iChunkX, int iChunkZ) {
    std::string strFileName = GetRegionFileName(iChunkX, iChunkZ);
    auto itr = m_mapOpenFiles.find(strFileName);
    if (itr != m_mapOpenFiles.end()) {
        return itr->second;
    }
    std::fstream* pFile = new std::fstream();
    if (!pFile)
        return nullptr;
    pFile->open(strFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!pFile->is_open()) {
        pFile->open(strFileName, std::ios::out | std::ios::binary);
        if (!pFile->is_open()) 
            return nullptr;
        std::vector<char> vecEmptyHeader(HEADER_SIZE, '\0');
        pFile->write(vecEmptyHeader.data(), HEADER_SIZE);
        pFile->close();
    }
    
    m_mapOpenFiles[strFileName] = pFile;
    return pFile;
}
// ********************************************************************
bool RegionManager::SaveChunk(const Chunk& objChunk) {
    std::lock_guard<std::mutex> lock(m_mutexIO);
    std::fstream* pFile = GetRegionFile(objChunk.GetChunkX(), objChunk.GetChunkZ());
    if (!pFile)
        return false;
    int iLocalX = objChunk.GetChunkX() & REGION_MASK;
    int iLocalZ = objChunk.GetChunkZ() & REGION_MASK;
    int iHeaderOffset = (iLocalX + iLocalZ * REGION_WIDTH) * 4;


    pFile->seekp(0, std::ios::end);
    int iFileEnd = static_cast<int>(pFile->tellp());
    int iDataSize = CHUNK_VOL;
    const uint8_t* pBlockData = objChunk.GetBlockData();
    pFile->write(reinterpret_cast<char*>(&iDataSize), sizeof(int));
    pFile->write(reinterpret_cast<const char*>(pBlockData), iDataSize);

    pFile->seekp(iHeaderOffset, std::ios::beg);
    pFile->write(reinterpret_cast<char*>(&iFileEnd), sizeof(int));

    return true;

}
// ********************************************************************
bool RegionManager::LoadChunk(Chunk& objChunk) {
    std::lock_guard<std::mutex> lock(m_mutexIO);
    std::fstream* pFile = GetRegionFile(objChunk.GetChunkX(), objChunk.GetChunkZ());
    if (!pFile)
        return false;
    int iLocalX = objChunk.GetChunkX() & REGION_MASK;
    int iLocalZ = objChunk.GetChunkZ() & REGION_MASK;
    int iHeaderOffset = (iLocalX + iLocalZ * REGION_WIDTH) * 4;

    pFile->seekg(iHeaderOffset, std::ios::beg);
    int iChunkOffset;
    pFile->read(reinterpret_cast<char*>(&iChunkOffset), sizeof(int));
    if (iChunkOffset == 0) {
        return false;
    }
    pFile->seekg(iChunkOffset, std::ios::beg);
    int iDataSize = 0;
    pFile->read(reinterpret_cast<char*>(&iDataSize), sizeof(int));
    if (iDataSize != CHUNK_VOL)
        return false;

    std::vector<uint8_t> vecChunkData(iDataSize);
    pFile->read(reinterpret_cast<char*>(vecChunkData.data()), iDataSize);
    objChunk.SetBlockData(vecChunkData.data());

    return true;
}
// ********************************************************************