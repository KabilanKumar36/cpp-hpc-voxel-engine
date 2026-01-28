#pragma once
#include <vector>
#include <cstdlib>
#include "../renderer/VertexArray.h"
#include "../renderer/IndexBuffer.h"
#include "../renderer/Buffer.h"
#include "../core/FastNoiseLite.h"

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 16;
constexpr int CHUNK_VOL = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
enum Direction {
	FRONT,
	BACK,
	LEFT,
	RIGHT,
	UP,
	DOWN,

};
class Chunk {
public:
	int m_iChunkX, m_iChunkZ;
	Chunk();
	Chunk(int iX, int iZ) : m_iChunkX(iX), m_iChunkZ(iZ){
		Init();
		noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
		noise.SetFrequency(0.04f);
		generateMesh();
	}
	void Init();
	~Chunk();
	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;

	Chunk(Chunk&& other) noexcept;
	Chunk& operator=(Chunk&& other) noexcept;

	void render();
	void setFaceCulling(bool bOpt) { m_bEnableFaceCulling = bOpt;  }
	bool getFaceCulling() { return m_bEnableFaceCulling;  }
private:
	bool m_bEnableFaceCulling;
	uint8_t m_iBlocks[CHUNK_VOL];
	FastNoiseLite noise;

	std::vector<float> m_vec_fVertices;
	std::vector<unsigned int> m_vec_uiIndices;
	Renderer::VertexArray* m_pVAO;
	Renderer::VertexBuffer* m_pVBO;
	Renderer::IndexBuffer* m_pIBO;

	void updateBuffers();
	void addBlockFace(int iX, int iY, int iZ, Direction iDir);
	void generateMesh();

};