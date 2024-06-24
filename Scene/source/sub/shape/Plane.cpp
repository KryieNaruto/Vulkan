#include "sub/shape/Plane.h"

#define QUAT 2

Core::Resource::Plane::Plane(const std::string& _name)
	:Shape(_name)
{
	m_scene_type = PLANE;
	m_type = "Plane";
	m_res_name = "Plane";
	// 该形状未被创建
	if (getVertex(m_type).size() == 0)
		generateModel(QUAT);
	setup();
}

Core::Resource::Plane::Plane()
	:Shape("Plane")
{
	m_scene_type = PLANE;
	m_type = "Plane";
	m_res_name = "Plane";
	// 该形状未被创建
	if (getVertex(m_type).size() == 0)
		generateModel(QUAT);
	setup();
}

Core::Resource::Plane::~Plane()
{
	
}

void Core::Resource::Plane::generateModel(uint32_t _quad)
{
	// 生成顶点
	if (_quad < 2) _quad = 2;
	m_quad = _quad;
	std::vector<uint32_t> p_indices;

	uint32_t v_count = m_quad * m_quad;
	uint32_t i_count = glm::pow(m_quad - 1, 2) * 6;
	p_indices.resize(i_count);

	float x_start = -0.5f;
	float z_start = -0.5f;
	float xz_offset = 1.0f / (m_quad - 1);
	// 正反面
	for (size_t x = 0; x < _quad; x++) {
		for (size_t z = 0; z < _quad; z++) {
			// VERTEX
			glm::vec3 pos;
			pos.x = x_start + xz_offset * x;
			pos.y = 0.0f;
			pos.z = z_start + xz_offset * z;

			glm::vec3 normal = glm::vec3(0, 1, 0);
			glm::vec2 texCoord;
			texCoord.x = xz_offset * x;
			texCoord.y = xz_offset * z;

			auto _title = "RVertex" + std::to_string(x) + std::to_string(z);
			RVertex* p_v = new RVertex(pos, normal, texCoord, _title);
			p_vertices.push_back(p_v);
		}
	}

	// INDICES
	uint32_t _index_start = 0;	// 索引起点
	uint32_t _index_index = 0;	// 四边形面片索引
	for (size_t i = 0; i < i_count; i += 6) {
		p_indices[i] = _index_start;
		p_indices[i + 1] = _index_start + m_quad;
		p_indices[i + 2] = _index_start + 1;
		p_indices[i + 3] = _index_start + 1;
		p_indices[i + 4] = _index_start + m_quad;
		p_indices[i + 5] = _index_start + m_quad + 1;
		_index_start++;
		if (++_index_index % (m_quad - 1) == 0) {
			_index_start = _index_start / (m_quad - 1) * m_quad;
		}
		generateTangentAndBitangent(p_indices, i);
		generateTangentAndBitangent(p_indices, i + 3);
	}

	saveVertex(m_type, p_vertices);
	saveIndex(m_type, p_indices);
}
