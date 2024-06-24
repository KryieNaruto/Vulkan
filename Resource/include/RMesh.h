#pragma once
#include "ZResource.h"
#include "RVertex.h"
#include "RMaterial.h"

namespace Core::Resource {
    
	/************************************************************************/
	// ֻ��������
	// ������ȫ���洢��RModel��
	/************************************************************************/
    class ENGINE_API_RESOURCE RMesh :
        public ZResource
    {
	public:
		RMesh(const int& _texture_index, const std::string& _name = "RMesh");
		RMesh(const std::vector<uint32_t>& _indices, const int& _texture_index, const std::string& _name = "RMesh");
		RMesh(const std::string& _name = "RMesh");
		virtual ~RMesh();
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& _root) override;

		void setMaterial(RMaterial* _material) { p_material = _material; setup(); }
		void update(RMeshProperty* _parent);
		virtual void draw(VkCommandBuffer _cmd);
		inline int getMaterialIndex() { return m_texture_index; }
		inline int getIndex() { return m_index; }
		inline void setIndex(int _index) { m_index = _index; }
		inline bool isHidden() { if (p_property) return p_property->m_hidden; else return false; }
		RMeshProperty* getProperty() { return p_property; }

		// �ϲ����mesh
		RMesh* mergeRMeshes(const std::vector<RMesh*>& _meshes);
	protected:
		// ��������
		std::vector<uint32_t> m_indices;
		ThirdParty::IndexBuffer* p_index_buffer = nullptr;

		RMaterial* p_material = nullptr;
		// ��������
		int m_texture_index = 0;
		// mesh ����
		int m_index = 0;

		RMeshProperty* p_property = nullptr;

	private:
		void setup();

		// ͨ�� ZResource �̳�
		void initTemplate() override;
	};

}

