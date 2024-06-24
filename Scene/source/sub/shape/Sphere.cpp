#include "sub/shape/Sphere.h"

void Core::Resource::Sphere::generateModel(uint32_t _quad)
{
	m_quad = _quad;
	float SPHERE_DIV = m_quad;
	float i, j;
	uint32_t p1, p2;
	auto pi = glm::pi<float>();
	float step = 2 * pi / SPHERE_DIV;
	// 顶点
	std::vector<uint32_t> p_indices;
	for (j = 0; j <= SPHERE_DIV; j++) {
		for (i = 0; i <= SPHERE_DIV; i++) {
			RVertex* _v = new RVertex();

			float xSegment = (float)i / (float)SPHERE_DIV;
			float ySegment = (float)j / (float)SPHERE_DIV;
			float xPos = std::cos(xSegment * 2.0f * pi) * std::sin(ySegment * pi);
			float yPos = std::cos(ySegment * pi);
			float zPos = std::sin(xSegment * 2.0f * pi) * std::sin(ySegment * pi);

			_v->m_position[0] = xPos / 2.0f;	// X
			_v->m_position[1] = yPos / 2.0f;	// Y
			_v->m_position[2] = zPos / 2.0f;	// Z

			_v->m_normal = glm::normalize(_v->m_position);

			float theta = atan2f(zPos, xPos);

			float phi = acosf(yPos);

			float u = theta / (2.0f * pi);
			float v = phi / pi;

			_v->m_texCoord[0] = xSegment;
			_v->m_texCoord[1] = ySegment;

			p_vertices.push_back(_v);
		}
	}

	// 索引
	for (j = 0; j < SPHERE_DIV; j++) {
		for (i = 0; i < SPHERE_DIV; i++) {
			p1 = j * (SPHERE_DIV + 1) + i;
			p2 = p1 + (SPHERE_DIV + 1);

			p_indices.push_back(p1);
			p_indices.push_back(p2);
			p_indices.push_back(p1 + 1);

			p_indices.push_back(p1 + 1);
			p_indices.push_back(p2);
			p_indices.push_back(p2 + 1);

			generateTangentAndBitangent({ p1,p2,p1 + 1 });
			generateTangentAndBitangent({ p1 + 1,p2,p2 + 1 });
		}
	}

	saveVertex(m_type, p_vertices);
	saveIndex(m_type, p_indices);
}



Core::Resource::Sphere::Sphere(const std::string& _name)
	:Shape(_name)
{
	m_scene_type = SPHERE;
	m_type = "Sphere";
	m_res_name = "Sphere";
	// 该形状未被创建
	auto& _vertices = getVertex(m_type);
	if (getVertex(m_type).empty())
		generateModel(25);
	setup();
}

Core::Resource::Sphere::Sphere()
	:Shape("Sphere")
{
	m_scene_type = SPHERE;
	m_type = "Sphere";
	m_res_name = "Sphere";
	// 该形状未被创建
	std::vector<RVertex*> _vertices(0);
	_vertices = getVertex(m_type);
	if (_vertices.empty())
		generateModel(25);
	setup();
}

Core::Resource::Sphere::~Sphere()
{

}
