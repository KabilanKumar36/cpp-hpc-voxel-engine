#include <gtest/gtest.h>
#include "../src/world/Chunk.h"

// --- Configuration & Layout Tests ---

TEST(ChunkTest, ConstantsCheck) {
    // Ensure chunk dimensions match the optimized 16^3 cubic volume
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
    // Verify Row-Major Cache Locality: X moves fastest, then Y, then Z.
    // Index = iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_HEIGHT)

    // Test X-Step (L1 Cache hit)
    EXPECT_EQ(Chunk(0, 0).GetFlatIndexOf3DLayer(1, 0, 0), 1);

    // Test Y-Step (Stride of 16)
    EXPECT_EQ(Chunk(0, 0).GetFlatIndexOf3DLayer(0, 1, 0), 16);

    // Test Z-Step (Stride of 256)
    EXPECT_EQ(Chunk(0, 0).GetFlatIndexOf3DLayer(0, 0, 1), 256);
}

// --- Memory & Ownership Tests (The "HPC Core") ---

TEST(ChunkTest, MoveConstructor_VAO_OwnershipTransfer) {
    // 1. Setup Source Chunk
    Chunk ObjSrcChunk(0, 0);

    // Requires OpenGL Context (Provided by test_main.cpp)
    ObjSrcChunk.ReconstructMesh();
    ObjSrcChunk.UploadMesh();

    ASSERT_TRUE(ObjSrcChunk.IsValid()) << "Setup failed: Source Chunk VAO not created";

    // 2. Move to Target
    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    // 3. Verify Ownership
    ASSERT_TRUE(ObjTgtChunk.IsValid()) << "Bug: Target Chunk received nullptr VAO";
    ASSERT_FALSE(ObjSrcChunk.IsValid())
        << "Bug: Source Chunk still holds VAO pointer (Double Free risk)";

    EXPECT_EQ(ObjTgtChunk.GetChunkX(), 0);

    // Requires OpenGL Context (Provided by test_main.cpp)
    ObjTgtChunk.ReconstructMesh();
    ObjTgtChunk.UploadMesh();
}

TEST(ChunkTest, MoveConstructor_ThermalBufferTransfer) {
    // Setup: Source owns 64-byte aligned thermal buffers and OpenGL pointers
    Chunk ObjSrcChunk(0, 0);

    // Capture pointers for verification before move
    float* pOriginalBuffer = ObjSrcChunk.GetCurrData();
    ASSERT_NE(pOriginalBuffer, nullptr);

    // Inject test data
    ObjSrcChunk.InjectHeat(8, 8, 8, 99.0f);

    // Act: Transfer ownership to Target
    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    // Assert: Target now owns the original memory address
    EXPECT_EQ(ObjTgtChunk.GetCurrData(), pOriginalBuffer);

    // Assert: Source is nulled to prevent 'other' destructor from calling _aligned_free
    EXPECT_EQ(ObjSrcChunk.GetCurrData(), nullptr);
    float fTemp = ObjTgtChunk.GetTemperatureAt(8, 8, 8);
    EXPECT_EQ(fTemp, 99.0f);
}

TEST(ChunkTest, MoveConstructor_ThermalDataIntegrity) {
    Chunk ObjSrcChunk(1, 1);

    // Inject 99.0f into the center voxel (8,8,8)
    ObjSrcChunk.InjectHeat(8, 8, 8, 99.0f);

    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    // Verify data survived the pointer transfer
    // We expect the exact float value at the same index
    int iIdx = ObjTgtChunk.GetFlatIndexOf3DLayer(8, 8, 8);
    EXPECT_FLOAT_EQ(ObjTgtChunk.GetCurrData()[iIdx], 99.0f);
}

TEST(ChunkTest, MoveAssignment_CleanupAndReplace) {
    Chunk ObjTgtChunk(0, 0);  // Allocate Buffer A
    Chunk ObjSrcChunk(1, 1);  // Allocate Buffer B

    // Capture Buffer B pointer
    float* pSrcBuffer = ObjSrcChunk.GetCurrData();

    // Act: Move Src into Tgt.
    // This MUST trigger _aligned_free on Buffer A to avoid a memory leak.
    ObjTgtChunk = std::move(ObjSrcChunk);

    EXPECT_EQ(ObjTgtChunk.GetCurrData(), pSrcBuffer);
    EXPECT_EQ(ObjSrcChunk.GetCurrData(), nullptr);
    EXPECT_EQ(ObjTgtChunk.GetChunkX(), 1);
}

TEST(ChunkThermalTest, BoundaryDiffusionZAxis) {
    // 1. Arrange: Create two adjacent chunks
    Chunk chunkHot(0, 0);
    Chunk chunkCold(0, 1);  // Z+1 is NORTH in our coordinate system

    // 2. Mock the Manager's linkage logic
    // chunkCold is NORTH of chunkHot. chunkHot is SOUTH of chunkCold.
    chunkHot.SetNeighbours(Direction::NORTH, &chunkCold);
    chunkCold.SetNeighbours(Direction::SOUTH, &chunkHot);

    // 3. Inject a massive heat spike on the absolute North edge of chunkHot (Z = 15)
    int testX = 8;
    int testY = 8;
    float startTemp = 5000.0f;
    chunkHot.InjectHeat(testX, testY, CHUNK_SIZE - 1, startTemp);

    // Verify initial state of Cold Chunk is 0.0f
    EXPECT_FLOAT_EQ(chunkCold.GetTemperatureAt(testX, testY, 0), 0.0f);

    // 4. Act: Step the physics engine once
    float fDiffusivity = 1.0f;  // Stable diffusivity to satisy Von Neumann stability criterion ( C
                                // (alpha * deltime) < 1/6)
    float fDeltaTime = 0.1f;
    chunkHot.ThermalStep(fDiffusivity, fDeltaTime);
    chunkHot.SwapBuffers();
    chunkCold.ThermalStep(fDiffusivity, fDeltaTime);
    chunkCold.SwapBuffers();

    // 5. Assert: Check if heat crossed the boundary into Z=0 of the Cold Chunk
    float fColdTemp = chunkCold.GetTemperatureAt(testX, testY, 0);

    // We expect the temperature to be strictly greater than 0
    EXPECT_GT(fColdTemp, 0.0f) << "Heat failed to cross the Z-axis boundary!";

    // Optional: Print the value to the test console for verification
    std::cout << "[          ] Transferred Heat: " << fColdTemp << std::endl;
}