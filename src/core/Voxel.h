#include <cstdlib>
#include <cstdint>

class Voxel {
	uint8_t  m_uiId;
	bool  m_bIsActive;

public:
	Voxel(uint8_t id =0, bool bActive = false) :
		m_uiId(id),
		m_bIsActive(bActive)
	{
	}
};