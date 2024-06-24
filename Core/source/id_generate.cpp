#include "id_generate.h"
#include <ThirdPartyManager/core/common/include/debug_to_console.h>
#include <chrono>
Core::IDGenerate::IDGenerate()
{
	this->m_bInit = false;
	this->m_addID = 0;
}

Core::IDGenerate::~IDGenerate()
{

}

void Core::IDGenerate::init(RESOURCE_TYPE resourceType)
{
	this->m_bInit = true;
	this->m_resourceType = (int)resourceType;
}

uint64_t Core::IDGenerate::generateUUID()
{
	if (!this->m_bInit) {
		ThirdParty::Core::warn("UUID has not been initialized");
		return 0;
	}
	uint32_t  currentSecond = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	// 若秒数不同了，则自增ID重新从0开始
	if (currentSecond != m_lastSecond)
	{
		m_lastSecond = currentSecond;
		m_addID = 0;
	}
	return ((uint64_t)m_resourceType << 59) + ((uint64_t)currentSecond << 20) + ((uint64_t)m_addID++);
}
