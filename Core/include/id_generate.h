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
		bool			m_bInit;        // �Ƿ��ѳ�ʼ��
		int				m_resourceType; // �ʲ�����
		int				m_lastSecond;   // �ϴβ���IDʱ��ʱ�������λ����
		int				m_addID;        // ����ID
	};

}

