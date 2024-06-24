#pragma once
#include <ThirdParty/glm/glm.hpp>
#include "core/info/include/application_window_info.h"
namespace ThirdParty {
	//求最大公约数
	int getCommonDivisor(int a, int b);
	//求最小公倍数
	int getCommonMultiple(int a, int b);
	// 求最小被除数, a: 被除数。 b: 除数
	ENGINE_API_THIRDPARTY int getMinusDividend(int a, int b);

	// 屏幕坐标转世界空间
	ENGINE_API_THIRDPARTY glm::vec3 calcScreenToWorld(const glm::vec2& _screen_pos, const glm::vec2& _screen_size, const glm::mat4& _proj_inverse, const glm::mat4& _view_inverse);
}