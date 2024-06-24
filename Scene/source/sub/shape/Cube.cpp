#include "sub/shape/Cube.h"

Core::Resource::Cube::Cube(const std::string& _name)
	:Shape(_name)
{
	m_scene_type = CUBE;
	m_type = "Cube";
	m_res_name = "Cube";
	// 该形状未被创建
	if (getVertex(m_type).size() == 0)
		generateModel(2);
	setup();
}

Core::Resource::Cube::Cube()
	:Shape("Cube")
{
	m_scene_type = CUBE;
	m_type = "Cube";
	m_res_name = "Cube";
	// 该形状未被创建
	if (getVertex(m_type).size() == 0)
		generateModel(2);
	setup();
}

void Core::Resource::Cube::generateModel(uint32_t _quad) {
	if (_quad < 2) _quad = 2;
	m_quad = _quad;
	std::vector<uint32_t> p_indices;

	uint32_t v_count = m_quad * m_quad;				// 每面顶点数
	uint32_t i_count = (m_quad - 1) * 6;			// 每面索引数
	uint32_t i_count_all = (m_quad - 1) * 6 * 6;	// 总的索引数
	p_indices.resize(i_count_all);

	uint32_t _index_count = 0;
	uint32_t _vertex_count = 0;
	// 前后面
	{
		float x_start = -0.5f;
		float y_start = -0.5f;
		float xy_offset = 2.0f / m_quad;
		for (int z = 0; z < 2; z++) {
			for (int x = 0; x < _quad; x++) {
				for (int y = 0; y < _quad; y++) {
					// VERTEX
					glm::vec3 pos;
					pos.x = x_start + xy_offset * x;
					pos.y = y_start + xy_offset * y;
					pos.z = 0.5f - z * 1;

					glm::vec3 normal = glm::vec3(0, 0, 1 - 2 * z);
					glm::vec2 texCoord;
					texCoord.x = xy_offset * x;
					texCoord.y = xy_offset * y;

					auto _title = "RVertex" + std::to_string(x) + std::to_string(z);
					RVertex* p_v = new RVertex(pos, normal, texCoord, _title);
					p_vertices.push_back(p_v);
				}
			}
			// INDICES
			for (int i = _index_count; i < i_count_all; i += 6) {
				if (z == 0) {
					p_indices[i] = _vertex_count;
					p_indices[i + 1] = _vertex_count + 1;
					p_indices[i + 2] = _vertex_count + m_quad;
					p_indices[i + 3] = _vertex_count + 1;
					p_indices[i + 4] = _vertex_count + m_quad + 1;
					p_indices[i + 5] = _vertex_count + m_quad;
				}
				else {
					p_indices[i] = _vertex_count;
					p_indices[i + 1] = _vertex_count + m_quad;
					p_indices[i + 2] = _vertex_count + 1;
					p_indices[i + 3] = _vertex_count + 1;
					p_indices[i + 4] = _vertex_count + m_quad;
					p_indices[i + 5] = _vertex_count + m_quad + 1;
				}
				generateTangentAndBitangent(p_indices, i);
				generateTangentAndBitangent(p_indices, i + 3);
			}
			_index_count += i_count;
			_vertex_count += v_count;
		}

	}

	// 左右面
	{
		float y_start = -0.5f;
		float z_start = -0.5f;
		float yz_offset = 2.0f / m_quad;
		for (int x = 0; x < 2; x++) {
			for (int y = 0; y < _quad; y++) {
				for (int z = 0; z < _quad; z++) {
					// VERTEX
					glm::vec3 pos;
					pos.x = 0.5f - x * 1;
					pos.y = y_start + yz_offset * y;
					pos.z = z_start + yz_offset * z;

					glm::vec3 normal = glm::vec3(1 - 2 * x, 0, 0);
					glm::vec2 texCoord;
					texCoord.x = yz_offset * y;
					texCoord.y = yz_offset * z;

					auto _title = "RVertex" + std::to_string(x) + std::to_string(z);
					RVertex* p_v = new RVertex(pos, normal, texCoord, _title);
					p_vertices.push_back(p_v);
				}
			}
			// INDICES
			for (int i = _index_count; i < i_count_all; i += 6) {
				if (x == 0) {
					p_indices[i] = _vertex_count;
					p_indices[i + 1] = _vertex_count + 1;
					p_indices[i + 2] = _vertex_count + m_quad;
					p_indices[i + 3] = _vertex_count + 1;
					p_indices[i + 4] = _vertex_count + m_quad + 1;
					p_indices[i + 5] = _vertex_count + m_quad;
				}
				else {
					p_indices[i] = _vertex_count;
					p_indices[i + 1] = _vertex_count + m_quad;
					p_indices[i + 2] = _vertex_count + 1;
					p_indices[i + 3] = _vertex_count + 1;
					p_indices[i + 4] = _vertex_count + m_quad;
					p_indices[i + 5] = _vertex_count + m_quad + 1;
				}
				generateTangentAndBitangent(p_indices, i);
				generateTangentAndBitangent(p_indices, i + 3);
			}
			_index_count += i_count;
			_vertex_count += v_count;
		}

	}

	// 上下面
	{
		float x_start = -0.5f;
		float z_start = -0.5f;
		float xz_offset = 2.0f / m_quad;
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < _quad; x++) {
				for (int z = 0; z < _quad; z++) {
					// VERTEX
					glm::vec3 pos;
					pos.x = x_start + xz_offset * x;
					pos.y = 0.5f - y * 1;
					pos.z = z_start + xz_offset * z;

					glm::vec3 normal = glm::vec3(0, 1 - 2 * y, 0);
					glm::vec2 texCoord;
					texCoord.x = xz_offset * x;
					texCoord.y = xz_offset * z;

					auto _title = "RVertex" + std::to_string(x) + std::to_string(z);
					RVertex* p_v = new RVertex(pos, normal, texCoord, _title);
					p_vertices.push_back(p_v);
				}
			}
			// INDICES
			for (int i = _index_count; i < i_count_all; i += 6) {
				if (y == 0) {
					p_indices[i] = _vertex_count;
					p_indices[i + 1] = _vertex_count + m_quad;
					p_indices[i + 2] = _vertex_count + 1;
					p_indices[i + 3] = _vertex_count + 1;
					p_indices[i + 4] = _vertex_count + m_quad;
					p_indices[i + 5] = _vertex_count + m_quad + 1;
				}
				else {
					p_indices[i] = _vertex_count;
					p_indices[i + 1] = _vertex_count + 1;
					p_indices[i + 2] = _vertex_count + m_quad;
					p_indices[i + 3] = _vertex_count + 1;
					p_indices[i + 4] = _vertex_count + m_quad + 1;
					p_indices[i + 5] = _vertex_count + m_quad;
				}
				generateTangentAndBitangent(p_indices, i);
				generateTangentAndBitangent(p_indices, i + 3);
			}
			_index_count += i_count;
			_vertex_count += v_count;
		}

	}

	saveVertex(m_type, p_vertices);
	saveIndex(m_type, p_indices);
}

Core::Resource::Cube::~Cube()
{

}


