#pragma once
#include <ThirdParty/glm/glm.hpp>
#include "core/info/include/application_window_info.h"
namespace ThirdParty {
	//�����Լ��
	int getCommonDivisor(int a, int b);
	//����С������
	int getCommonMultiple(int a, int b);
	// ����С������, a: �������� b: ����
	ENGINE_API_THIRDPARTY int getMinusDividend(int a, int b);

	// ��Ļ����ת����ռ�
	ENGINE_API_THIRDPARTY glm::vec3 calcScreenToWorld(const glm::vec2& _screen_pos, const glm::vec2& _screen_size, const glm::mat4& _proj_inverse, const glm::mat4& _view_inverse);
}