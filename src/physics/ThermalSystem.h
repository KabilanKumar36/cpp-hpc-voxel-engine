#pragma once

#include <atomic>
#include <barrier>
#include <memory>
#include <thread>
#include <vector>

class ChunkManager;
// ********************************************************************
class ThermalSystem {
public:
    ThermalSystem(int iNumThreads);
    ~ThermalSystem();

    void UpdateTemperature(float fDeltaTime, ChunkManager& objChunkManager);

private:
    void workerThreadLoop(int iThreadID);

    int m_iNumThreads;
    std::atomic<bool> m_bIsRunning;

    float m_fCurrDeltaTime;
    ChunkManager* m_pCurrChunkManager;

    std::unique_ptr<std::barrier<>> m_pStartBarrier;
    std::unique_ptr<std::barrier<>> m_pPhase1Barrier;
    std::unique_ptr<std::barrier<>> m_pPhase2Barrier;

    std::vector<std::thread> m_vecWorkerThreads;
};