#include "RVertex.h"

Core::Resource::RVertex::RVertex(const std::string& _name /*= "RVertex"*/)
{
	this->m_res_name = _name;
	this->m_json_title = "RVertex";
	this->p_id_generate->init(RESOURCE_TYPE::RVERTEX);
	this->m_uuid = this->p_id_generate->generateUUID();
}

Core::Resource::RVertex::RVertex
(glm::vec3 _pos, glm::vec3 _normal, glm::vec2 _texCoord, glm::vec3 _tangent, glm::vec3 _bitangent, const std::string& _name /*= "RVertex"*/)
	:m_position(_pos),m_normal(_normal),m_texCoord(_texCoord),m_tangent(_tangent),m_bitangent(_bitangent)
{
	this->m_res_name = _name;
	this->m_json_title = "RVertex";
	this->p_id_generate->init(RESOURCE_TYPE::RVERTEX);
	this->m_uuid = this->p_id_generate->generateUUID();
}

Core::Resource::RVertex::RVertex()
{
	this->m_res_name = "RVertex";
	this->m_json_title = "RVertex";
	this->p_id_generate->init(RESOURCE_TYPE::RVERTEX);
	this->m_uuid = this->p_id_generate->generateUUID();
}

Core::Resource::RVertex::RVertex(glm::vec3 _pos, glm::vec3 _normal, glm::vec2 _texCoord, const std::string& _name /*= "RVertex"*/, glm::vec3 _tangent /*= {0,0,0}*/, glm::vec3 _bitangent /*= {0,0,0}*/)
	:m_position(_pos), m_normal(_normal), m_texCoord(_texCoord), m_tangent(_tangent), m_bitangent(_bitangent)
{
	this->m_res_name = _name;
	this->m_json_title = "RVertex";
	this->p_id_generate->init(RESOURCE_TYPE::RVERTEX);
	this->m_uuid = this->p_id_generate->generateUUID();
}

Core::Resource::RVertex::~RVertex()
{

}

Json::Value Core::Resource::RVertex::serializeToJSON()
{
	return Json::Value();
}

void Core::Resource::RVertex::deserializeToObj(Json::Value& _root)
{

}


std::variant<
	Core::Resource::VertexInput_vec3_1,
	Core::Resource::VertexInput_default,
	Core::Resource::VertexInput_default_T_B
> Core::Resource::RVertex::getData(VERTEX_INPUT_FLAG _flag)
{
	std::variant<
		Core::Resource::VertexInput_vec3_1,
		Core::Resource::VertexInput_default,
		Core::Resource::VertexInput_default_T_B
	> combination;
	switch (_flag)
	{
	case Core::Resource::_vec3_1:
		return VertexInput_vec3_1{ m_position };
	case Core::Resource::_default:
		return VertexInput_default{ m_position ,m_normal,m_texCoord };
	case Core::Resource::_default_T_B:
		return VertexInput_default_T_B{ m_position,m_normal,m_texCoord,m_tangent,m_bitangent };
	default:
		break;
	}
}

#define CALCULATE_VERTEXINPUT_SIZE(value) sizeof(VertexInput##value);
void Core::Resource::RVertex::pushData(VERTEX_INPUT_FLAG _flag, void* _dst, size_t& _offset)
{
	// void* 指针操作与char* 一致
	auto dst = (char*)_dst + _offset;
	switch (_flag)
	{
	case Core::Resource::_vec2_1: {
		memcpy(dst, &m_texCoord, sizeof(glm::vec2));
		_offset += CALCULATE_VERTEXINPUT_SIZE(_vec2_1);
		break;
	}
	case Core::Resource::_vec3_1: {
		memcpy(dst, &m_position, sizeof(glm::vec3));
		_offset += CALCULATE_VERTEXINPUT_SIZE(_vec3_1);
		break;
	}
	case Core::Resource::_default:
	// switch 中， 如果有初始化struct的操作，需要用{}限定作用域
	{
		size_t _size = CALCULATE_VERTEXINPUT_SIZE(_default)
		VertexInput_default _v_in = { m_position,m_normal,m_texCoord };
		memcpy_s(dst, _size, &_v_in, _size);
		_offset += _size;
		break;
	}
	case Core::Resource::_vec3_1_vec2_2:
	{
		size_t _size = CALCULATE_VERTEXINPUT_SIZE(_vec3_1_vec2_2);
		VertexInput_vec3_1_vec2_2 _v_in = { m_position,m_texCoord };
		memcpy_s(dst, _size, &_v_in, _size);
		_offset += _size;
		break;
	}
	case Core::Resource::_default_T:
	{
		size_t _size = CALCULATE_VERTEXINPUT_SIZE(_default_T);
		VertexInput_default_T _v_in = { m_position,m_normal,m_texCoord,m_tangent };
		memcpy_s(dst, _size, &_v_in, _size);
		_offset += _size;
		break;
	}
	case Core::Resource::_default_T_B:
	{
		size_t _size = CALCULATE_VERTEXINPUT_SIZE(_default_T_B);
		VertexInput_default_T_B _v_in = { m_position,m_normal,m_texCoord,m_tangent,m_bitangent };
		memcpy_s(dst, _size, &_v_in, _size);
		_offset += _size;
	}
	default:
		break;
	}
}

void Core::Resource::RVertex::initTemplate()
{
}
