/**
 * @file ThermalSystem.cpp
 * @brief Implementation of the multi-threaded ThermalSystem utilizing std::barrier.
 */

#include "ThermalSystem.h"
#include <algorithm>
#include "../world/Chunk.h"
#include "../world/ChunkManager.h"

// ********************************************************************
ThermalSystem::ThermalSystem(int iNumThreads)
    : m_iNumThreads(iNumThreads),
      m_bIsRunning(true),
      m_bIsSIMDEnabled(true),
      m_fCurrDeltaTime(0.0f),
      m_pCurrChunkManager(nullptr) {
    int iTotalParticipants = m_iNumThreads + 1;  // +1 for main thread
    m_pStartBarrier = std::make_unique<std::barrier<>>(iTotalParticipants);

    m_pPhase1Barrier = std::make_unique<std::barrier<>>(iNumThreads);
    m_pPhase2Barrier = std::make_unique<std::barrier<>>(iTotalParticipants);

    for (int i = 0; i < m_iNumThreads; ++i) {
        m_vecWorkerThreads.emplace_back(&ThermalSystem::workerThreadLoop, this, i);
    }
}
// ********************************************************************
ThermalSystem::~ThermalSystem() {
    m_bIsRunning = false;
    m_pStartBarrier->arrive_and_wait();
    for (auto& thread : m_vecWorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
// ********************************************************************
void ThermalSystem::UpdateTemperature(float fDeltaTime, ChunkManager& objChunkManager) {
    m_fCurrDeltaTime = fDeltaTime;
    m_pCurrChunkManager = &objChunkManager;

    m_pStartBarrier->arrive_and_wait();
    m_pPhase2Barrier->arrive_and_wait();
}
// ********************************************************************
void ThermalSystem::workerThreadLoop(int iThreadID) {
    const float fAlpha = 0.2f;
    const float fDelX = 1.0f;
    // Using the CFL(Courant-Friedrichs-Lewy) stability condition
    // For 3D explicit diffusion it is: fDeltaTime <= (fDelX^2) / (6 * fAlpha)
    const float fMaxDeltaTime = (fDelX * fDelX) / (6.0f * fAlpha);  // CFL Condition

    while (m_bIsRunning) {
        m_pStartBarrier->arrive_and_wait();
        if (!m_bIsRunning)
            break;
        int iNbSteps = 0;
        float fStepTime = 0.0f;
        if (m_fCurrDeltaTime > 0.0f) {
            iNbSteps = static_cast<int>(std::ceil(m_fCurrDeltaTime / fMaxDeltaTime));
            fStepTime = m_fCurrDeltaTime / static_cast<float>(iNbSteps);
        }
        for (int iStepCt = 0; iStepCt < iNbSteps; iStepCt++) {
            auto& chunks = m_pCurrChunkManager->GetMutableChunks();
            int iChunkIndex = 0;
            for (auto& [coords, pChunk] : chunks) {
                if (iChunkIndex % m_iNumThreads == iThreadID) {
                    if (m_bIsSIMDEnabled)
                        pChunk->ThermalStep_AVX2(fStepTime, fAlpha);
                    else
                        pChunk->ThermalStep(fStepTime, fAlpha);
                }
                ++iChunkIndex;
            }
            m_pPhase1Barrier->arrive_and_wait();

            iChunkIndex = 0;
            for (auto& [coords, pChunk] : chunks) {
                if (iChunkIndex % m_iNumThreads == iThreadID) {
                    pChunk->SwapBuffers();
                }
                ++iChunkIndex;
            }

            if (iStepCt < iNbSteps - 1)
                m_pPhase1Barrier->arrive_and_wait();
        }

        m_pPhase2Barrier->arrive_and_wait();
    }
}
// ********************************************************************