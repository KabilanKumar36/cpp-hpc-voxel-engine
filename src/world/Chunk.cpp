#include "Chunk.h"
#include <iostream>
#include <cstring>
//*********************************************************************
Chunk::~Chunk() {
	if (m_pVAO) delete m_pVAO;
	if (m_pVBO) delete m_pVBO;
	if (m_pIBO) delete m_pIBO;
}
//*********************************************************************
Chunk::Chunk(Chunk&& other) noexcept : 
m_vec_fVertices(std::move(other.m_vec_fVertices)), 
m_vec_uiIndices(std::move(other.m_vec_uiIndices)), m_pVAO(other.m_pVAO), 
m_pVBO(other.m_pVBO), m_pIBO(other.m_pIBO), m_iChunkX(other.m_iChunkX), 
m_iChunkZ(other.m_iChunkZ), m_bEnableFaceCulling(other.m_bEnableFaceCulling)
{
	m_pVAO = nullptr;
	m_pVBO = nullptr;
	m_pIBO = nullptr;

	std::memcpy(m_iBlocks, other.m_iBlocks, sizeof(m_iBlocks));

}
//*********************************************************************
Chunk& Chunk::operator=(Chunk&& other) noexcept
{
	if (this != &other)
	{
		m_vec_fVertices = std::move(other.m_vec_fVertices);
		m_vec_uiIndices = std::move(other.m_vec_uiIndices);

		if(m_pVAO) delete m_pVAO;
		if(m_pVBO) delete m_pVBO;
		if(m_pIBO) delete m_pIBO;

		m_pVAO = other.m_pVAO;
		m_pVBO = other.m_pVBO;
		m_pIBO = other.m_pIBO;

		other.m_pVAO = nullptr;
		other.m_pVBO = nullptr;
		other.m_pIBO = nullptr;

		m_iChunkX = other.m_iChunkX;
		m_iChunkZ = other.m_iChunkZ;

		std::memcpy(m_iBlocks, other.m_iBlocks, sizeof(m_iBlocks));
	}
	return *this;
}
//*********************************************************************
void Chunk::updateBuffers() {

	if (m_pVBO) { delete m_pVBO; m_pVBO = nullptr; }
	if (m_pIBO) { delete m_pIBO; m_pIBO = nullptr; }
	if (m_vec_fVertices.size())
		m_pVBO = new Renderer::VertexBuffer(m_vec_fVertices.data(),
			static_cast<unsigned int>(m_vec_fVertices.size()) * sizeof(float));
	if (m_vec_uiIndices.size())
	{
		m_pIBO = new Renderer::IndexBuffer(m_vec_uiIndices.data(),
			static_cast<unsigned int>(m_vec_uiIndices.size()));
	}
	if (m_pVBO)
	{
		m_pVAO->linkAttribute(*m_pVBO, 0, 3, 5, 0);
		m_pVAO->linkAttribute(*m_pVBO, 1, 2, 5, 3);
		if (m_pIBO) m_pIBO->Bind();
		if (m_pVAO) m_pVAO->Unbind();
		if (m_pIBO) m_pIBO->Unbind();
	}

	std::cout << "Chunk Mesh Updated: " << m_vec_fVertices.size() / 5 << " vertices." << std::endl;
}
//*********************************************************************
void Chunk::addBlockFace(int iX, int iY, int iZ, Direction iDir, int iBlockType) {
	unsigned int iStartIndex = static_cast<unsigned int>(m_vec_fVertices.size()) / 5;
	float fX = static_cast<float>(iX) + (CHUNK_SIZE * m_iChunkX);
	float fY = static_cast<float>(iY);
	float fZ = static_cast<float>(iZ) + (CHUNK_SIZE * m_iChunkZ);

	/* DEBUG PRINT : Check if this actually prints different numbers!
	if (iY == 0 && iX == 0 && iZ == 0) {
		std::cout << "Chunk [" << m_iChunkX << "," << m_iChunkZ << "] WorldX: " << fX << std::endl;
	}*/

	int iAtlasCol = 10, iAtlasRow = 8;
	if (iBlockType == 1)//Grass
	{
		if (iDir == Direction::UP)//Grass
		{
			 iAtlasCol = 2; iAtlasRow = 9; 
		}
		else if (iDir == Direction::DOWN) //Stone
		{
			iAtlasCol = 1; iAtlasRow = 0;
		}
		else //Grass + Dirt
		{
			iAtlasCol = 3; iAtlasRow = 0;
		}
	}
	else if (iBlockType == 2) //Dirt
	{
		if (iDir == Direction::DOWN) //Stone
		{
			iAtlasCol = 1; iAtlasRow = 0;
		}
		else //Dirt
		{
			iAtlasCol = 2; iAtlasRow = 0;
		}
	}
	else if (iBlockType == 3) //Stone
	{
		iAtlasCol = 1; iAtlasRow = 0;
	}
	float fSlotSize = 1.0f / 16.0f;
	float u0 = iAtlasCol * fSlotSize;
	float v1 = iAtlasRow * fSlotSize;
	float u1 = u0 + fSlotSize;
	float v0 = v1 + fSlotSize;


	switch (iDir)
	{
	case Direction::FRONT: // Z+ Face
	{
		// Origin: Bottom-Left (x, y)
		// Winding: Right (+x) -> Up (+y) -> Left (-x)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,		fZ + 1, u0, v0, // 0: Bottom-Left
			fX + 1, fY,		fZ + 1, u1, v0, // 1: Bottom-Right
			fX + 1, fY + 1, fZ + 1, u1, v1, // 2: Top-Right
			fX,		fY + 1, fZ + 1, u0, v1, // 3: Top-Left
			});
	}break;

	case Direction::BACK: // Z- Face
	{
		// Origin: Bottom-Right (x, y) if looking from back, but (x,y) in World Coords
		// Winding: Up (+y) -> Right (+x) -> Down (-y)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,		fZ,		u0, v0, // 0: Bottom-Right (from back view)
			fX,		fY + 1,	fZ,		u0, v1, // 1: Top-Right
			fX + 1, fY + 1, fZ,		u1, v1, // 2: Top-Left
			fX + 1,	fY,		fZ,		u1, v0, // 3: Bottom-Left
			});
	}break;

	case Direction::RIGHT: // X+ Face
	{
		// Origin: Bottom-Front (y, z)
		// Winding: Up (+y) -> Back (+z) -> Down (-y)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX + 1,	fY,		fZ,		u0, v0, // 0: Bottom-Front
			fX + 1,	fY + 1,	fZ,		u0, v1, // 1: Top-Front
			fX + 1, fY + 1, fZ + 1, u1, v1, // 2: Top-Back
			fX + 1,	fY,		fZ + 1, u1, v0, // 3: Bottom-Back
			});
	}break;

	case Direction::LEFT: // X- Face
	{
		// Origin: Bottom-Back (y, z)
		// Winding: Back (+z) -> Up (+y) -> Front (-z)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,		fZ,		u0, v0, // 0: Bottom-Back
			fX,		fY,		fZ + 1,	u1, v0, // 1: Bottom-Front
			fX,		fY + 1, fZ + 1, u1, v1, // 2: Top-Front
			fX,		fY + 1,	fZ,		u0, v1, // 3: Top-Back
			});
	}break;

	case Direction::UP: // Y+ Face
	{
		// Origin: Top-Left-Front (x, z)
		// Winding: Back (+z) -> Right (+x) -> Front (-z)
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY + 1,	fZ,		u0, v1, // 0: Back-Left
			fX,		fY + 1,	fZ + 1,	u0, v0, // 1: Front-Left
			fX + 1,	fY + 1, fZ + 1, u1, v0, // 2: Front-Right
			fX + 1,	fY + 1,	fZ,		u1, v1, // 3: Back-Right
			});
	}break;

	case Direction::DOWN: // Y- Face
	{
		// Origin: Bottom-Left-Back (x, z)
		// Winding: Right (+x) -> Front (+z) -> Left (-x)
		// FIX: Corrected typo from previous code snippet
		m_vec_fVertices.insert(m_vec_fVertices.end(), {
			fX,		fY,	fZ,			u0, v1, // 0: Back-Left
			fX + 1,	fY,	fZ,			u1, v1, // 1: Back-Right
			fX + 1,	fY, fZ + 1,		u1, v0, // 2: Front-Right
			fX,		fY,	fZ + 1,		u0, v0, // 3: Front-Left
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
void Chunk::updateHeightData(){
	noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	noise.SetFrequency(0.04f);
	for (int iX = 0; iX < CHUNK_SIZE; iX++)
	{
		for (int iZ = 0; iZ < CHUNK_SIZE; iZ++)
		{
			int iWorldX = (m_iChunkX * CHUNK_SIZE) + iX;
			int iWorldZ = (m_iChunkZ * CHUNK_SIZE) + iZ;
			float fNoiseVal = noise.GetNoise(static_cast<float>(iWorldX),
				static_cast<float>(iWorldZ));
			int iHeight = static_cast<int>((fNoiseVal + 1.0f) * 10.0f);
			if (iHeight < 0) iHeight = 0;
			if (iHeight >= CHUNK_HEIGHT) iHeight = CHUNK_HEIGHT - 1;
			m_iHeightData[iX][iZ] = iHeight;

			for (int iY = 0; iY <= iHeight; iY++)
			{
				int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
				
				uint8_t iBlockType = 3;								//Stone
				if (iY == iHeight) iBlockType = 1;				//Grass
				else if (iY > iHeight - 3) iBlockType = 2;		//Dirt							
				
				m_iBlocks[iIndex] = iBlockType;
			}
		}
	}
}
//*********************************************************************
void Chunk::GenerateMesh() {
	m_vec_fVertices.clear();
	m_vec_uiIndices.clear();
	for (int iX = 0; iX < CHUNK_SIZE; iX++)
	{
		for (int iZ = 0; iZ < CHUNK_SIZE; iZ++)
		{
			for (int iY = 0; iY <= m_iHeightData[iX][iZ]; iY++)
			{
				int iIndex = GetFlatIndexOf3DLayer(iX, iY, iZ);
				int iBlockType = m_iBlocks[iIndex];
				if (iBlockType == 0)
					continue;
				if (iY == m_iHeightData[iX][iZ] || m_iBlocks[GetFlatIndexOf3DLayer(iX, iY + 1, iZ)] == 0)
					addBlockFace(iX, iY, iZ, Direction::UP, iBlockType);
				if (iY == 0 || m_iBlocks[GetFlatIndexOf3DLayer(iX, iY - 1, iZ)] == 0)
					addBlockFace(iX, iY, iZ, Direction::DOWN, iBlockType);

				if (iX == CHUNK_SIZE - 1 || m_iBlocks[GetFlatIndexOf3DLayer(iX + 1, iY, iZ)] == 0)
					addBlockFace(iX, iY, iZ, Direction::RIGHT, iBlockType);
				if (iX == 0 || m_iBlocks[GetFlatIndexOf3DLayer(iX - 1, iY, iZ)] == 0)
					addBlockFace(iX, iY, iZ, Direction::LEFT, iBlockType);

				if (iZ == CHUNK_SIZE - 1 || m_iBlocks[GetFlatIndexOf3DLayer(iX, iY, iZ + 1)] == 0)
					addBlockFace(iX, iY, iZ, Direction::FRONT, iBlockType);
				if (iZ == 0 || m_iBlocks[GetFlatIndexOf3DLayer(iX, iY, iZ - 1)] == 0)
					addBlockFace(iX, iY, iZ, Direction::BACK, iBlockType);
			}
		}
	}
	updateBuffers();
}
//*********************************************************************
void Chunk::Render() const {
	if (m_pVAO && m_pVBO && m_pIBO)
	{
		m_pVAO->Bind();
		glDrawElements(GL_TRIANGLES, m_pIBO->m_uiCount, GL_UNSIGNED_INT, 0);
		m_pVAO->Unbind();
	}
}
//*********************************************************************