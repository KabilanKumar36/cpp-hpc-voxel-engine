#include <gtest/gtest.h>
#include "../src/world/Chunk.h"

TEST(ChunkTest, MoveConstructor_OwnershipTransfer){
    Chunk ObjSrcChunk(0, 0);

    ObjSrcChunk.ReconstructMesh();
    ObjSrcChunk.UploadMesh();

    ASSERT_TRUE( ObjSrcChunk.IsValid()) << "Setup failed, Source Chunk didn't create VAO Found";
    Chunk ObjTgtChunk(std::move(ObjSrcChunk));

    ASSERT_TRUE( ObjTgtChunk.IsValid()) << "Bug Detected: Target Chunk received nullptr for VAO";

    ASSERT_FALSE( ObjSrcChunk.IsValid()) << "Bug Detected: Source Chunk still has the VAO Pointer";

    EXPECT_EQ(ObjTgtChunk.GetChunkX(), 0);
}