#include "math_util.h"

int ThirdParty::getCommonDivisor(int a, int b)
{
	int c = 0;
	while ((c = a % b))
	{
		a = b;
		b = c;
	}
	return b;
}

int ThirdParty::getCommonMultiple(int a, int b)
{
	auto _div = getCommonDivisor(a, b);
	auto _rel = a * b / _div;
	return _rel;
}

int ThirdParty::getMinusDividend(int a, int b)
{
	int _factor = a / b + 1;
	return b * _factor;
}

glm::vec3 ThirdParty::calcScreenToWorld(const glm::vec2& _screen_pos, const glm::vec2& _screen_size, const glm::mat4& _proj_inverse, const glm::mat4& _view_inverse)
{
	// фад╩ -> NDC
	glm::vec2 _ndc = _screen_pos / _screen_size;
	_ndc = _ndc * glm::vec2(2.0f) - glm::vec2(1.0f);
	// NDC  -> Clip
	glm::vec4 _clip = glm::vec4(_ndc, -1.0f, 1.0f);
	// Clip -> View
	glm::vec4 _view = _proj_inverse * _clip;
	_view = glm::vec4(_view.x, _view.y, -1.0f, 0.0f);
	// View -> World
	return _view_inverse * _view;
}

