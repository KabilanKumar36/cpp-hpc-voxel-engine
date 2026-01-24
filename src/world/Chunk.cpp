#include "Chunk.h"
#include <iostream>
#define FACE_CULLING 1
Chunk::Chunk() {
		// Initialize blocks with random data (for testing)
	for (int i = 0; i < CHUNK_VOL; ++i)
	{
		m_iBlocks[i] = (i % 2 == 0) ? 1 : 0; //1; //for solid block check
	}
	m_pVAO = new Renderer::VertexArray();
	m_pVBO = NULL;
	m_bEnableFaceCulling = false;
}
//*********************************************************************
Chunk::~Chunk() {
	if (m_pVAO) delete m_pVAO;
	if (m_pVBO) delete m_pVBO;
}
//*********************************************************************
bool Chunk::shouldRenderFace(int iXInd, int iYInd, int iZInd)
{
	if (iXInd < 0 || iXInd >= CHUNK_SIZE || iYInd < 0 || iYInd >= CHUNK_SIZE ||
		iZInd < 0 || iZInd >= CHUNK_SIZE)
		return true;
	int iIndex = iXInd + (iYInd * CHUNK_SIZE) + (iZInd * CHUNK_SIZE * CHUNK_SIZE);
	if (iIndex < 0 || iIndex >= CHUNK_VOL)
		return true;
	return m_iBlocks[iIndex] == 0;
}
//*********************************************************************
void Chunk::addFace(float x, float y, float z, const float *fFaceVertices) {
	for (int i = 0; i < 6; ++i)
	{
		//xyz coords
		m_vec_fVertices.push_back(fFaceVertices[i * 5 + 0] + x);
		m_vec_fVertices.push_back(fFaceVertices[i * 5 + 1] + y);
		m_vec_fVertices.push_back(fFaceVertices[i * 5 + 2] + z);
		
		//tetxture uv coords (0 to 1)
		m_vec_fVertices.push_back(fFaceVertices[i * 5 + 3]);
		m_vec_fVertices.push_back(fFaceVertices[i * 5 + 4]);
	}
}
//*********************************************************************
void Chunk::updateMesh() {
	m_vec_fVertices.clear();
	// Define face vertices (position + texture coords)

	float fCubeFaceCoords[6][30] = {
		// Front Face (z + 1)
		{ 0,0,1, 0,0,  1,0,1, 1,0,  1,1,1, 1,1,  1,1,1, 1,1,  0,1,1, 0,1,  0,0,1, 0,0 },
		// Back Face (z + 0)
		{ 1,0,0, 0,0,  0,0,0, 1,0,  0,1,0, 1,1,  0,1,0, 1,1,  1,1,0, 0,1,  1,0,0, 0,0 },
		// Right Face (x + 1)
		{ 1,0,1, 0,0,  1,0,0, 1,0,  1,1,0, 1,1,  1,1,0, 1,1,  1,1,1, 0,1,  1,0,1, 0,0 },
		// Left Face (x + 0)
		{ 0,0,0, 0,0,  0,0,1, 1,0,  0,1,1, 1,1,  0,1,1, 1,1,  0,1,0, 0,1,  0,0,0, 0,0 },
		// Top Face (y + 1)
		{ 0,1,1, 0,0,  1,1,1, 1,0,  1,1,0, 1,1,  1,1,0, 1,1,  0,1,0, 0,1,  0,1,1, 0,0 },
		// Bottom Face (y + 0)
		{ 0,0,0, 0,0,  1,0,0, 1,0,  1,0,1, 1,1,  1,0,1, 1,1,  0,0,1, 0,1,  0,0,0, 0,0 }
	};
	for (size_t iXCtr = 0; iXCtr < CHUNK_SIZE; iXCtr++)
	{
		for (size_t iYCtr = 0; iYCtr < CHUNK_SIZE; iYCtr++)
		{
			for (size_t iZCtr = 0; iZCtr < CHUNK_SIZE; iZCtr++)
			{
				int iIndex = iXCtr + (iYCtr * CHUNK_SIZE) + (iZCtr * CHUNK_SIZE * CHUNK_SIZE);

				if (m_iBlocks[iIndex]) {
					if (m_bEnableFaceCulling)
					{
						if (shouldRenderFace(iXCtr, iYCtr, iZCtr + 1))
							addFace(static_cast<float>(iXCtr), static_cast<float>(iYCtr),
								static_cast<float>(iZCtr), fCubeFaceCoords[0]);

						if (shouldRenderFace(iXCtr, iYCtr, iZCtr - 1))
							addFace(static_cast<float>(iXCtr), static_cast<float>(iYCtr),
								static_cast<float>(iZCtr), fCubeFaceCoords[1]);

						if (shouldRenderFace(iXCtr + 1, iYCtr, iZCtr))
							addFace(static_cast<float>(iXCtr), static_cast<float>(iYCtr),
								static_cast<float>(iZCtr), fCubeFaceCoords[2]);

						if (shouldRenderFace(iXCtr - 1, iYCtr, iZCtr))
							addFace(static_cast<float>(iXCtr), static_cast<float>(iYCtr),
								static_cast<float>(iZCtr), fCubeFaceCoords[3]);

						if (shouldRenderFace(iXCtr, iYCtr + 1, iZCtr))
							addFace(static_cast<float>(iXCtr), static_cast<float>(iYCtr),
								static_cast<float>(iZCtr), fCubeFaceCoords[4]);

						if (shouldRenderFace(iXCtr, iYCtr - 1, iZCtr))
							addFace(static_cast<float>(iXCtr), static_cast<float>(iYCtr),
								static_cast<float>(iZCtr), fCubeFaceCoords[5]);
					}
					else
					{
						// With out Face Culling
						for (size_t iFaceCtr = 0; iFaceCtr < 6; iFaceCtr++)
						{
							addFace(static_cast<float>(iXCtr), static_cast<float>(iYCtr),
								static_cast<float>(iZCtr), fCubeFaceCoords[iFaceCtr]);
						}
					}
				}
			}
		}
	}
	if (m_pVBO) { delete m_pVBO; m_pVBO = NULL; }
	m_pVBO = new Renderer::VertexBuffer(m_vec_fVertices.data(),
		m_vec_fVertices.size() * sizeof(float));

	m_pVAO->linkAttribute(*m_pVBO, 0, 3, 5, 0);
	m_pVAO->linkAttribute(*m_pVBO, 1, 2, 5, 3);

	std::cout << "Chunk Mesh Updated: " << m_vec_fVertices.size() / 5 << " vertices." << std::endl;
}
//*********************************************************************
void Chunk::render() {
	if (m_pVAO && m_pVBO)
	{
		m_pVAO->bind();
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vec_fVertices.size() / 5));
		m_pVAO->unbind();
	}
}
//*********************************************************************