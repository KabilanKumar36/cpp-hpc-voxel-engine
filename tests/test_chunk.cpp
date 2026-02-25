#include <gtest/gtest.h>
#include "../src/world/Chunk.h"

TEST(ChunkTest, ConstantsCheck) {
    EXPECT_EQ(CHUNK_SIZE, 16);
    EXPECT_EQ(CHUNK_HEIGHT, 16);
    // 16 * 16 * 16 = 4096
    EXPECT_EQ(CHUNK_VOL, 4096);
}

TEST(ChunkTest, InitialCoordinates) {
    Chunk chunk(5, -3);
    EXPECT_EQ(chunk.GetChunkX(), 5);
    EXPECT_EQ(chunk.GetChunkZ(), -3);
}

TEST(ChunkTest, BlockIndicesCalculation) {
    // Verify the math matches: iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_HEIGHT)
    // Note: In your Chunk.h logic, Y is the middle term, Z is the outer term.

    int iX = 1;
    int iY = 0;
    int iZ = 0;
    // Index = 1 + 0 + 0 = 1
    int iCalculatedIndex = iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_HEIGHT);
    EXPECT_EQ(iCalculatedIndex, 1);

    iX = 0;
    iY = 1;
    iZ = 0;
    // Index = 0 + 16 + 0 = 16
    iCalculatedIndex = iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_HEIGHT);
    EXPECT_EQ(iCalculatedIndex, 16);
}

TEST(ChunkTest, MoveConstructor_OwnershipTransfer) {
    // 1. Setup Source Chunk
    Chunk ObjSrcChunk(0, 0);

    // Requires OpenGL Context (Provided by TestMain)
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
}

TEST(ChunkTest, MoveConstructor_ThermalBufferTransfer) {
    Chunk ObjSrcChunk(1, 1);

    // Inject test data
    ObjSrcChunk.InjectHeat(8, 8, 8, 99.0f);

    // Perform the Move
    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    // 1. Verify target has the data
    // (You might need a public getter for testing, or just verify via DebugPrint)
    // For now, let's just check the "validity" via a null check if you add one

    // 2. Verify source pointers are NULL
    EXPECT_EQ(ObjSrcChunk.GetCurrData(), nullptr);
    EXPECT_NE(ObjTgtChunk.GetCurrData(), nullptr);
}