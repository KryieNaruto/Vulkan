#pragma once
#include <cstdint>
#include "core_struct.h"

namespace Core {

	class ENGINE_API_CORE IDGenerate
	{
	public:
		IDGenerate();
		virtual ~IDGenerate();
		void init(RESOURCE_TYPE resourceType);
		uint64_t generateUUID();
	private:
	protected:
		bool			m_bInit;        // 是否已初始化
		int				m_resourceType; // 资产类型
		int				m_lastSecond;   // 上次产生ID时的时间戳，单位：秒
		int				m_addID;        // 自增ID
	};

}

