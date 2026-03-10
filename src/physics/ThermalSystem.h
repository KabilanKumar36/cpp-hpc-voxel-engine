/**
 * @file ThermalSystem.h
 * @brief Defines the ThermalSystem class for multi-threaded, SIMD-accelerated heat diffusion.
 */

#pragma once

#include <atomic>
#include <barrier>
#include <memory>
#include <thread>
#include <vector>

class ChunkManager;
// ********************************************************************

/**
 * @class ThermalSystem
 * @brief Manages a thread pool to compute 3D explicit thermal diffusion across voxel chunks.
 */
class ThermalSystem {
public:
    ThermalSystem(int iNumThreads);
    ~ThermalSystem();

    /**
     * @brief Wakes worker threads to compute the next thermal integration step based on elapsed
     * time.
     */
    void UpdateTemperature(float fDeltaTime, ChunkManager& objChunkManager);
    void SetEnableSIMD(bool bEnable) { m_bIsSIMDEnabled = bEnable; }

private:
    /**
     * @brief Main execution loop for each worker thread, regulated by barrier synchronization.
     */
    void workerThreadLoop(int iThreadID);

    int m_iNumThreads;
    std::atomic<bool> m_bIsRunning;
    std::atomic<bool> m_bIsSIMDEnabled;

    float m_fCurrDeltaTime;
    ChunkManager* m_pCurrChunkManager;

    std::unique_ptr<std::barrier<>> m_pStartBarrier;
    std::unique_ptr<std::barrier<>> m_pPhase1Barrier;
    std::unique_ptr<std::barrier<>> m_pPhase2Barrier;

    std::vector<std::thread> m_vecWorkerThreads;
};