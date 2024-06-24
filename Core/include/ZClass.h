#pragma once
#include <cstdint>
#include "id_generate.h"

namespace Core {
	class ENGINE_API_CORE ZClass
	{
	public:
		ZClass() = default;
		virtual ~ZClass() = 0;
		const uint64_t& getUUID() { return m_uuid; }
	private:
	protected:
		IDGenerate* p_id_generate = nullptr;
		uint64_t m_uuid = 0;
		bool m_initialized = false;
	};
}


