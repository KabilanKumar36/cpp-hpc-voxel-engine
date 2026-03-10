/**
 * @file ChunkTest.cpp
 * @brief Google Test suite for the Chunk class, verifying memory ownership, move semantics, and
 * thermal diffusion boundaries.
 */

#include <gtest/gtest.h>
#include "../src/world/Chunk.h"

// --- Configuration & Layout Tests ---

TEST(ChunkTest, ConstantsCheck) {
    EXPECT_EQ(CHUNK_SIZE, 16);
    EXPECT_EQ(CHUNK_HEIGHT, 16);
    EXPECT_EQ(CHUNK_VOL, 4096);
}

TEST(ChunkTest, InitialCoordinates) {
    Chunk chunk(5, -3);
    EXPECT_EQ(chunk.GetChunkX(), 5);
    EXPECT_EQ(chunk.GetChunkZ(), -3);
}

TEST(ChunkTest, BlockIndicesCalculation) {
    EXPECT_EQ(Chunk(0, 0).GetFlatIndexOf3DLayer(1, 0, 0), 1);
    EXPECT_EQ(Chunk(0, 0).GetFlatIndexOf3DLayer(0, 1, 0), 16);
    EXPECT_EQ(Chunk(0, 0).GetFlatIndexOf3DLayer(0, 0, 1), 256);
}

// --- Memory & Ownership Tests (The "HPC Core") ---

TEST(ChunkTest, MoveConstructor_VAO_OwnershipTransfer) {
    Chunk ObjSrcChunk(0, 0);

    // Requires OpenGL Context (Provided by test_main.cpp)
    ObjSrcChunk.ReconstructMesh();
    ObjSrcChunk.UploadMesh();

    ASSERT_TRUE(ObjSrcChunk.IsValid()) << "Setup failed: Source Chunk VAO not created";

    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    ASSERT_TRUE(ObjTgtChunk.IsValid()) << "Bug: Target Chunk received nullptr VAO";
    ASSERT_FALSE(ObjSrcChunk.IsValid())
        << "Bug: Source Chunk still holds VAO pointer (Double Free risk)";

    EXPECT_EQ(ObjTgtChunk.GetChunkX(), 0);

    // Requires OpenGL Context (Provided by test_main.cpp)
    ObjTgtChunk.ReconstructMesh();
    ObjTgtChunk.UploadMesh();
}

TEST(ChunkTest, MoveConstructor_ThermalBufferTransfer) {
    Chunk ObjSrcChunk(0, 0);

    float* pOriginalBuffer = ObjSrcChunk.GetCurrData();
    ASSERT_NE(pOriginalBuffer, nullptr);

    ObjSrcChunk.InjectHeat(8, 8, 8, 99.0f);

    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    EXPECT_EQ(ObjTgtChunk.GetCurrData(), pOriginalBuffer);
    EXPECT_EQ(ObjSrcChunk.GetCurrData(), nullptr);

    float fTemp = ObjTgtChunk.GetTemperatureAt(8, 8, 8);
    EXPECT_EQ(fTemp, 99.0f);
}

TEST(ChunkTest, MoveConstructor_ThermalDataIntegrity) {
    Chunk ObjSrcChunk(1, 1);
    ObjSrcChunk.InjectHeat(8, 8, 8, 99.0f);

    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    EXPECT_FLOAT_EQ(ObjTgtChunk.GetTemperatureAt(8, 8, 8), 99.0f);
}

TEST(ChunkTest, MoveAssignment_CleanupAndReplace) {
    Chunk ObjTgtChunk(0, 0);
    Chunk ObjSrcChunk(1, 1);

    float* pSrcBuffer = ObjSrcChunk.GetCurrData();

    ObjTgtChunk = std::move(ObjSrcChunk);

    EXPECT_EQ(ObjTgtChunk.GetCurrData(), pSrcBuffer);
    EXPECT_EQ(ObjSrcChunk.GetCurrData(), nullptr);
    EXPECT_EQ(ObjTgtChunk.GetChunkX(), 1);
}

// --- Physics & Simulation Tests ---

TEST(ChunkThermalTest, BoundaryDiffusionZAxis) {
    Chunk chunkHot(0, 0);
    Chunk chunkCold(0, 1);  // Z+1 is NORTH in our coordinate system

    chunkHot.SetNeighbours(Direction::NORTH, &chunkCold);
    chunkCold.SetNeighbours(Direction::SOUTH, &chunkHot);

    int testX = 8;
    int testY = 8;
    float startTemp = 5000.0f;
    chunkHot.InjectHeat(testX, testY, CHUNK_SIZE - 1, startTemp);

    EXPECT_FLOAT_EQ(chunkCold.GetTemperatureAt(testX, testY, 0), 0.0f);

    // Stable diffusivity to satisfy Von Neumann stability criterion (C (alpha * deltime) < 1/6)
    float fDiffusivity = 1.0f;
    float fDeltaTime = 0.1f;
    chunkHot.ThermalStep(fDiffusivity, fDeltaTime);
    chunkCold.ThermalStep(fDiffusivity, fDeltaTime);
    chunkHot.SwapBuffers();
    chunkCold.SwapBuffers();

    float fColdTemp = chunkCold.GetTemperatureAt(testX, testY, 0);

    EXPECT_GT(fColdTemp, 0.0f) << "Heat failed to cross the Z-axis boundary!";

    std::cout << "[          ] Transferred Heat: " << fColdTemp << std::endl;
}