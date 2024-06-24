#include "model/RShape.h"

void Core::Resource::RShape::generateModel(uint32_t _quad)
{
	m_quad = _quad;
}

void Core::Resource::RShape::saveVertex(const std::string& _type, const std::vector<RVertex*>& _v)
{
	g_vertices[_type] = _v;
}


void Core::Resource::RShape::saveIndex(const std::string& _type, const std::vector<uint32_t>& _v)
{
	g_indices[_type] = _v;
}

const std::vector<Core::Resource::RVertex*>& Core::Resource::RShape::getVertex(const std::string& _type)
{
	std::vector<Core::Resource::RVertex*> _rel;
	_rel.resize(0);
	if (g_vertices.find(_type) != g_vertices.end())
		return g_vertices[_type];
	else return _rel;
}

const std::vector<uint32_t>& Core::Resource::RShape::getIndex(const std::string& _type)
{
	std::vector<uint32_t> _rel;
	_rel.resize(0);
	if (g_indices.find(_type) != g_indices.end())
		return g_indices[_type];
	else return _rel;
}

Core::Resource::RShape::RShape(const std::string& _name)
{
	g_shape_count++;
	m_res_name = _name;
	// 如果名称存在，则修改
	uint32_t index = 0;
	Core::Core_Pool::addResource(m_uuid, this);
}

Core::Resource::RShape::~RShape()
{
	// 取消销毁pvertex， 转而统一销毁
	g_shape_count--;
	p_vertices.clear();
	if (g_shape_count == 0) {
		// 统一销毁pvertex
		for (const auto& _v : g_vertices)
			for (const auto& v : _v.second)
				delete v;
	}
}

void Core::Resource::RShape::setup()
{
	auto p_indices = getIndex(m_type);
	p_vertices = getVertex(m_type);
	// 将p_vertices p_indices 封装成RMesh
	RMesh* p_mesh = new RMesh(p_indices, 0, "RMesh");
	p_meshes.push_back(p_mesh);
	p_textures[0] = {};
	m_initialized = true;
}

const std::vector<glm::vec3>& Core::Resource::RShape::calculateTangentAndBitangent(const glm::vec3 _pos[3], const glm::vec2 _uv[3])
{
	glm::vec3 edge1 = _pos[1] - _pos[0];
	glm::vec3 edge2 = _pos[2] - _pos[0];
	glm::vec2 deltaUV1 = _uv[1] - _uv[0];
	glm::vec2 deltaUV2 = _uv[2] - _uv[0];

	glm::vec3 tangent;
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent = glm::normalize(tangent);

	glm::vec3 bitTangent;
	bitTangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitTangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitTangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitTangent = glm::normalize(bitTangent);

	return { tangent,bitTangent };
}

void Core::Resource::RShape::generateTangentAndBitangent(int _indices[3])
{
	auto pos1 = p_vertices[_indices[0]];
	auto pos2 = p_vertices[_indices[0]];
	auto pos3 = p_vertices[_indices[0]];
	// 计算切线
	const glm::vec3 _pos[] = {
		pos1->m_position,
		pos2->m_position,
		pos3->m_position,
	};
	const glm::vec2 _uv[] = {
		pos1->m_texCoord,
		pos2->m_texCoord,
		pos3->m_texCoord,
	};
	auto _TB1 = calculateTangentAndBitangent(_pos, _uv);
	{
		pos1->m_tangent = _TB1[0];
		pos1->m_bitangent = _TB1[1];
		pos2->m_tangent = _TB1[0];
		pos2->m_bitangent = _TB1[1];
		pos3->m_tangent = _TB1[0];
		pos3->m_bitangent = _TB1[1];
	}
}

void Core::Resource::RShape::generateTangentAndBitangent(const std::vector<uint32_t>& _indices, int _indices_start)
{
	int t_indices[3] = {
		_indices[_indices_start],
		_indices[_indices_start + 1],
		_indices[_indices_start + 2],
	};
	generateTangentAndBitangent(t_indices);
}
