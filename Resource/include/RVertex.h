#pragma once
#include "ZResource.h"
#include <ThirdParty/glm/glm.hpp>
#include "resource_inner_global.h"
#include <variant>
namespace Core::Resource {

	class ENGINE_API_RESOURCE RVertex :
		public ZResource
	{
	public:
		RVertex();
		RVertex(glm::vec3 _pos, glm::vec3 _normal, glm::vec2 _texCoord, glm::vec3 _tangent = { 0,0,0 }, glm::vec3 _bitangent = { 0,0,0 }, const std::string& _name = "RVertex");
		RVertex(glm::vec3 _pos, glm::vec3 _normal, glm::vec2 _texCoord, const std::string& _name = "RVertex", glm::vec3 _tangent = {0,0,0}, glm::vec3 _bitangent = {0,0,0});
		RVertex(const std::string& _name);
		virtual ~RVertex();
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& _root) override;

		[[deprecated]]
		std::variant<
			VertexInput_vec3_1,
			VertexInput_default,
			VertexInput_default_T_B
		> getData(VERTEX_INPUT_FLAG _flag);	// ÆúÓÃ

		void pushData(VERTEX_INPUT_FLAG _flag, void* _dst, size_t& _offset);

		bool operator==(const RVertex& _vertex) {
			return m_position == _vertex.m_position && m_normal == _vertex.m_normal && m_texCoord == _vertex.m_texCoord;
		}
		bool operator==(RVertex* _vertex) {
			return m_position == _vertex->m_position && m_normal == _vertex->m_normal && m_texCoord == _vertex->m_texCoord;
		}
	public:
		glm::vec3 m_position;
		glm::vec3 m_normal;
		glm::vec2 m_texCoord;
		glm::vec3 m_tangent;
		glm::vec3 m_bitangent;

		// Í¨¹ý ZResource ¼Ì³Ð
		void initTemplate() override;
	};
}

