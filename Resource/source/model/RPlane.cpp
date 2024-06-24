#include "model/RPlane.h"

void Core::Resource::RPlane::generateModel(uint32_t _quad)
{
	// 生成顶点
	if (_quad < 2) _quad = 2;
	m_quad = _quad;
	std::vector<uint32_t> p_indices;
	std::vector<RVertex*> p_vertices;

	uint32_t v_count = m_quad * m_quad;
	uint32_t i_count = (m_quad - 1) * 6;
	p_indices.resize(i_count);

	float x_start = -1.0f;
	float z_start = -1.0f;
	float xz_offset = 2.0f / m_quad;
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
	for (size_t i = 0; i < i_count; i += 6) {
		p_indices[i] = i;
		p_indices[i + 1] = i + m_quad;
		p_indices[i + 2] = i + 1;
		p_indices[i + 3] = i + 1;
		p_indices[i + 4] = i + m_quad;
		p_indices[i + 5] = i + m_quad + 1;
		// 计算切线
		const glm::vec3 pos1[] = {
			p_vertices[i]->m_position,
			p_vertices[i + m_quad]->m_position,
			p_vertices[i + 1]->m_position,
		};
		const glm::vec2 uv1[] = {
			p_vertices[i]->m_texCoord,
			p_vertices[i + m_quad]->m_texCoord,
			p_vertices[i + 1]->m_texCoord,
		};
		auto _TB1 = calculateTangentAndBitangent(pos1, uv1);
		{
			p_vertices[i]->m_tangent = _TB1[0];
			p_vertices[i]->m_bitangent = _TB1[1];
			p_vertices[i + m_quad]->m_tangent = _TB1[0];
			p_vertices[i + m_quad]->m_bitangent = _TB1[1];
			p_vertices[i + 1]->m_tangent = _TB1[0];
			p_vertices[i + 1]->m_bitangent = _TB1[1];
		}

		const glm::vec3 pos2[] = {
			p_vertices[i + 1]->m_position,
			p_vertices[i + m_quad]->m_position,
			p_vertices[i + m_quad + 1]->m_position,
		};
		const glm::vec2 uv2[] = {
			p_vertices[i + 1]->m_texCoord,
			p_vertices[i + m_quad]->m_texCoord,
			p_vertices[i + m_quad + 1]->m_texCoord,
		};
		auto _TB2 = calculateTangentAndBitangent(pos2, uv2);
		{
			p_vertices[i + 1]->m_tangent = _TB2[0];
			p_vertices[i + 1]->m_bitangent = _TB2[1];
			p_vertices[i + m_quad]->m_tangent = _TB2[0];
			p_vertices[i + m_quad]->m_bitangent = _TB2[1];
			p_vertices[i + m_quad + 1]->m_tangent = _TB2[0];
			p_vertices[i + m_quad + 1]->m_bitangent = _TB2[1];
		}
	}

	saveVertex(m_type, p_vertices);
	saveIndex(m_type, p_indices);
};

Core::Resource::RPlane::RPlane()
	:RShape("RPlane")
{
	m_type = "RPlane";
	m_res_name = "RPlane";
	// 该形状未被创建
	if (getVertex(m_type).size() == 0)
		generateModel(2);
	else {
		p_vertices = getVertex(m_type);
	}
	setup();
};

Core::Resource::RPlane::RPlane(const std::string& _name)
	:RShape(_name)
{
	m_type = "RPlane";
	// 该形状未被创建
	if (getVertex(m_type).size() == 0)
		generateModel(2);
	else {
		p_vertices = getVertex(m_type);
	}
	setup();
}

Core::Resource::RPlane::~RPlane()
{
	
}
