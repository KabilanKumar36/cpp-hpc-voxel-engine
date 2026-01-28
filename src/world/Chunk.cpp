#include "Chunk.h"
#include <iostream>
#define FACE_CULLING 1
Chunk::Chunk() {
	m_iChunkX = 0;
	m_iChunkZ = 0;
		// Initialize blocks with random data (for testing)
	for (int i = 0; i < CHUNK_VOL; ++i)
	{
		m_iBlocks[i] = (i % 2 == 0) ? 1 : 0; //1; //for solid block check
	}
	Init();
}
//*********************************************************************
void Chunk::Init(){
	m_pVAO = new Renderer::VertexArray();
	m_pVBO = NULL;
	m_pIBO = NULL;
	m_bEnableFaceCulling = false;
}
//*********************************************************************
Chunk::~Chunk() {
	if (m_pVAO) delete m_pVAO;
	if (m_pVBO) delete m_pVBO;
	if (m_pIBO) delete m_pIBO;
}
//*********************************************************************
Chunk::Chunk(Chunk&& other) noexcept : m_bEnableFaceCulling(other.m_bEnableFaceCulling),
m_iChunkX(other.m_iChunkX), m_iChunkZ(other.m_iChunkZ), m_pVAO(other.m_pVAO), m_pVBO(other.m_pVBO),
m_pIBO(other.m_pIBO), m_vec_fVertices(std::move(other.m_vec_fVertices)), 
m_vec_uiIndices(std::move(other.m_vec_uiIndices))
{
	m_pVAO = NULL;
	m_pVBO = NULL;
	m_pIBO = NULL;

	std::memcpy(m_iBlocks, other.m_iBlocks, CHUNK_VOL);

}
//*********************************************************************
Chunk& Chunk::operator=(Chunk&& other) noexcept
{
	if (this != &other)
	{
		delete m_pVAO;
		delete m_pVBO;
		delete m_pIBO;

		m_pVAO = other.m_pVAO;
		m_pVBO = other.m_pVBO;
		m_pIBO = other.m_pIBO;

		other.m_pVAO = NULL;
		other.m_pVBO = NULL;
		other.m_pIBO = NULL;

		m_vec_fVertices = std::move(other.m_vec_fVertices);
		m_vec_uiIndices = std::move(other.m_vec_uiIndices);
		std::memcpy(m_iBlocks, other.m_iBlocks, CHUNK_VOL);
	}
	return *this;
}
//*********************************************************************
void Chunk::updateBuffers() {

	if (m_pVBO) { delete m_pVBO; m_pVBO = NULL; }
	if (m_pIBO) { delete m_pIBO; m_pIBO = NULL; }
	if (m_vec_fVertices.size())
		m_pVBO = new Renderer::VertexBuffer(m_vec_fVertices.data(),
			m_vec_fVertices.size() * sizeof(float));
	if (m_vec_uiIndices.size())
	{
		m_pIBO = new Renderer::IndexBuffer(m_vec_uiIndices.data(),
			static_cast<unsigned int>(m_vec_uiIndices.size()));
	}
	if (m_pVBO)
	{
		m_pVAO->linkAttribute(*m_pVBO, 0, 3, 5, 0);
		m_pVAO->linkAttribute(*m_pVBO, 1, 2, 5, 3);
		if (m_pIBO) m_pIBO->bind();
		if (m_pVAO) m_pVAO->unbind();
		if (m_pIBO) m_pIBO->unbind();
	}

	std::cout << "Chunk Mesh Updated: " << m_vec_fVertices.size() / 5 << " vertices." << std::endl;
}
//*********************************************************************
void Chunk::addBlockFace(int iX, int iY, int iZ, Direction iDir) {
	unsigned int iStartIndex = m_vec_fVertices.size() / 5;
	float fX = static_cast<float>(iX) + (CHUNK_SIZE * m_iChunkX);
	float fY = static_cast<float>(iY);
	float fZ = static_cast<float>(iZ) + (CHUNK_SIZE * m_iChunkZ);

	/* DEBUG PRINT : Check if this actually prints different numbers!
	if (iY == 0 && iX == 0 && iZ == 0) {
		std::cout << "Chunk [" << m_iChunkX << "," << m_iChunkZ << "] WorldX: " << fX << std::endl;
	}*/
	switch (iDir)
	{
	case Direction::FRONT: // Z+ Face
	{
		// Origin: Bottom-Left (x, y)
		// Winding: Right (+x) -> Up (+y) -> Left (-x)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,		fZ + 1, 0.0f, 0.0f, // 0: Bottom-Left
			fX + 1, fY,		fZ + 1, 1.0f, 0.0f, // 1: Bottom-Right
			fX + 1, fY + 1, fZ + 1, 1.0f, 1.0f, // 2: Top-Right
			fX,		fY + 1, fZ + 1, 0.0f, 1.0f, // 3: Top-Left
			});
	}break;

	case Direction::BACK: // Z- Face
	{
		// Origin: Bottom-Right (x, y) if looking from back, but (x,y) in World Coords
		// Winding: Up (+y) -> Right (+x) -> Down (-y)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,		fZ,		0.0f, 0.0f, // 0: Bottom-Right (from back view)
			fX,		fY + 1,	fZ,		1.0f, 0.0f, // 1: Top-Right
			fX + 1, fY + 1, fZ,		1.0f, 1.0f, // 2: Top-Left
			fX + 1,	fY,		fZ,		0.0f, 1.0f, // 3: Bottom-Left
			});
	}break;

	case Direction::RIGHT: // X+ Face
	{
		// Origin: Bottom-Front (y, z)
		// Winding: Up (+y) -> Back (+z) -> Down (-y)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX + 1,	fY,		fZ,		0.0f, 0.0f, // 0: Bottom-Front
			fX + 1,	fY + 1,	fZ,		1.0f, 0.0f, // 1: Top-Front
			fX + 1, fY + 1, fZ + 1, 1.0f, 1.0f, // 2: Top-Back
			fX + 1,	fY,		fZ + 1, 0.0f, 1.0f, // 3: Bottom-Back
			});
	}break;

	case Direction::LEFT: // X- Face
	{
		// Origin: Bottom-Back (y, z)
		// Winding: Back (+z) -> Up (+y) -> Front (-z)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,		fZ,		0.0f, 0.0f, // 0: Bottom-Back
			fX,		fY,		fZ + 1,	1.0f, 0.0f, // 1: Bottom-Front
			fX,		fY + 1, fZ + 1, 1.0f, 1.0f, // 2: Top-Front
			fX,		fY + 1,	fZ,		0.0f, 1.0f, // 3: Top-Back
			});
	}break;

	case Direction::UP: // Y+ Face
	{
		// Origin: Top-Left-Front (x, z)
		// Winding: Back (+z) -> Right (+x) -> Front (-z)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY + 1,	fZ,		0.0f, 0.0f, // 0: Back-Left
			fX,		fY + 1,	fZ + 1,	1.0f, 0.0f, // 1: Front-Left
			fX + 1,	fY + 1, fZ + 1, 1.0f, 1.0f, // 2: Front-Right
			fX + 1,	fY + 1,	fZ,		0.0f, 1.0f, // 3: Back-Right
			});
	}break;

	case Direction::DOWN: // Y- Face
	{
		// Origin: Bottom-Left-Back (x, z)
		// Winding: Right (+x) -> Front (+z) -> Left (-x)
		// FIX: Corrected typo from previous code snippet
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,	fZ,			0.0f, 0.0f, // 0: Back-Left
			fX + 1,	fY,	fZ,			1.0f, 0.0f, // 1: Back-Right
			fX + 1,	fY, fZ + 1,		1.0f, 1.0f, // 2: Front-Right
			fX,		fY,	fZ + 1,		0.0f, 1.0f, // 3: Front-Left
			});
	}break;
	default: break;
	}

	// Indices for two triangles (CCW)
	// Tri 1: 0 -> 1 -> 2
	m_vec_uiIndices.push_back(iStartIndex + 0);
	m_vec_uiIndices.push_back(iStartIndex + 1);
	m_vec_uiIndices.push_back(iStartIndex + 2);

	// Tri 2: 2 -> 3 -> 0
	m_vec_uiIndices.push_back(iStartIndex + 2);
	m_vec_uiIndices.push_back(iStartIndex + 3);
	m_vec_uiIndices.push_back(iStartIndex + 0);
}
//*********************************************************************
void Chunk::generateMesh() {
	m_vec_fVertices.clear();
	m_vec_uiIndices.clear();
	int iHeightData[CHUNK_SIZE][CHUNK_SIZE];
	for (size_t iX = 0; iX < CHUNK_SIZE; iX++)
	{
		for (size_t iZ = 0; iZ < CHUNK_SIZE; iZ++)
		{
			int iWorldX = (m_iChunkX * CHUNK_SIZE) + iX;
			int iWorldZ = (m_iChunkZ * CHUNK_SIZE) + iZ;
			float fNoiseVal = noise.GetNoise(static_cast<float>(iWorldX),
				static_cast<float>(iWorldZ));
			int iHeight = static_cast<int>((fNoiseVal + 1.0f) * 10.0f);
			if (iHeight < 0) iHeight = 0;
			if (iHeight >= CHUNK_HEIGHT) iHeight = CHUNK_HEIGHT - 1;

			for (size_t iY = 0; iY <= CHUNK_HEIGHT; iY++)
			{
				int iBlockType = 0;
				if (iY == iHeight) iBlockType = 1;				//Grass
				else if (iY > iHeight - 3) iBlockType = 2;		//Dirt
				else iBlockType = 3;							//Stone

				int iIndex = iX + (iY * CHUNK_SIZE) + (iZ * CHUNK_SIZE * CHUNK_SIZE);
				m_iBlocks[iIndex] = iBlockType;
			}
			iHeightData[iX][iZ] = iHeight;
		}
	}

	for (size_t iX = 0; iX < CHUNK_SIZE; iX++)
	{
		for (size_t iZ = 0; iZ < CHUNK_SIZE; iZ++)
		{
			for (size_t iY = 0; iY <= iHeightData[iX][iZ]; iY++)
			{
				if (iY == iHeightData[iX][iZ])
					addBlockFace(iX, iY, iZ, Direction::UP);
				if (iY == 0)
					addBlockFace(iX, iY, iZ, Direction::DOWN);

				if (iX == CHUNK_SIZE - 1 || iHeightData[iX + 1][iZ] < iY)
					addBlockFace(iX, iY, iZ, Direction::RIGHT);
				if (iX == 0 || iHeightData[iX - 1][iZ] < iY)
					addBlockFace(iX, iY, iZ, Direction::LEFT);

				if (iZ == CHUNK_SIZE - 1 || iHeightData[iX][iZ + 1] < iY)
					addBlockFace(iX, iY, iZ, Direction::FRONT);
				if (iZ == 0 || iHeightData[iX][iZ - 1] < iY)
					addBlockFace(iX, iY, iZ, Direction::BACK);
			}
		}
	}
	updateBuffers();
}
//*********************************************************************
void Chunk::render() {
	if (m_pVAO && m_pVBO && m_pIBO)
	{
		m_pVAO->bind();
		glDrawElements(GL_TRIANGLES, m_pIBO->m_uiCount, GL_UNSIGNED_INT, 0);
		m_pVAO->unbind();
	}
}
//*********************************************************************