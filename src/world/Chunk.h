#pragma once
#include <vector>
#include <cstdlib>
#include "../renderer/VertexArray.h"
#include "../renderer/Buffer.h"

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_VOL = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

class Chunk {
public:
	Chunk();
	~Chunk();

	void updateMesh();
	void render();
	void setFaceCulling(bool bOpt) { m_bEnableFaceCulling = bOpt;  }
	bool getFaceCulling() { return m_bEnableFaceCulling;  }
private:
	bool m_bEnableFaceCulling;
	uint8_t m_iBlocks[CHUNK_VOL];

	std::vector<float> m_vec_fVertices;
	Renderer::VertexArray* m_pVAO;
	Renderer::VertexBuffer* m_pVBO;

	bool shouldRenderFace(int iXInd, int iYInd, int iZInd);
	void addFace(float x, float y, float z, const float *fFaceVertices);
};