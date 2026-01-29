#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>
#include "../src/world/Chunk.h"
#include <iostream>

class OpenGLEnv : public ::testing::Environment{
public:
	GLFWwindow* pWindow;

	void SetUp() override {
		if (!glfwInit()) {
			std::cerr << "FATAL: Failed to init GLFW for unit tests." << std::endl;
			exit(-1);
		}

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		pWindow = glfwCreateWindow(600, 460, "Hidden test window", NULL, NULL);
		if (!pWindow) {
			std::cerr << "FATAL: Failed to create GLFW Window for unit tests." << std::endl;
			exit(-1);
		}
		glfwMakeContextCurrent(pWindow);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cerr << "FATAL: Failed to load GLAD for unit tests." << std::endl;
			exit(-1);
		}
	}

	void TearDown() override {
		glfwDestroyWindow(pWindow);
		glfwTerminate();
	}
};
TEST(ChunkTest, InitialCoordinates) {
	Chunk chunk(5, -3);
	EXPECT_EQ(chunk.m_iChunkX, 5);
	EXPECT_EQ(chunk.m_iChunkZ, -3);
}

TEST(ChunkTest, ConstantsCheck) {
	EXPECT_EQ(CHUNK_SIZE, 16);
	EXPECT_EQ(CHUNK_HEIGHT, 16);
	EXPECT_EQ(CHUNK_VOL, 4096);
}
TEST(ChunkTest, BlockIndicies) {
	int iX = 0, iY = 0, iZ = 0;
	int iIndex = iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_SIZE);
	EXPECT_EQ(iIndex, 0);

	iX = 1;
	EXPECT_EQ(iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_SIZE), 1);

}
int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	::testing::AddGlobalTestEnvironment(new OpenGLEnv);

	return RUN_ALL_TESTS();
}

int GetChunckCoord(int iPos) {
	int iChunckSize = 16;
	if (iPos >= 0) {
		return iPos / iChunckSize;
	}
	else {
		return (iPos - iChunckSize + 1) / iChunckSize;
	}
}

int GetLocalCoord(int iPos) {
	int iChunckSize = 16;
	int iLocalPos = iPos % iChunckSize;
	if (iLocalPos < 0)
		iLocalPos += iChunckSize;
	return iLocalPos;
}
