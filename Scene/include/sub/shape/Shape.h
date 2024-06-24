#pragma once
#include <Resource/resource_global.h>
#include "../scene_obj.h"
#include "../Model.h"
namespace Core::Resource {

    class ENGINE_API_SCENE Shape:
        public Model
    {
    public:
        Shape(const std::string& _name);
        virtual ~Shape();
		virtual void draw(VkCommandBuffer _cmd,Shader_Data_Binding_Func _func);
        virtual void draw(SceneObject* _scene, Material* _mat, VkCommandBuffer _cmd, Shader_Data_Binding_Func _func, void* _camera = nullptr);
		// ���ݰ����ⲿ�����
        virtual void draw(VkCommandBuffer _cmd);
		virtual void propertyEditor() override;
        virtual void generateModel(uint32_t _quad);
        virtual void setup();
    protected:
        inline static uint32_t g_shape_count = 0;
        std::string m_type = "Shape";
        uint32_t m_quad = 0;

		// ��������, ��������������һ��
		void generateTangentAndBitangent(const std::vector<uint32_t>& _indices);
		// ��������, �����������,��������
		void generateTangentAndBitangent(const std::vector<uint32_t>& _indices, int _indices_start);
		// ��������
		auto calculateTangentAndBitangent(const glm::vec3 _pos[3], const glm::vec2 _uv[3]);
        // ��˼�������
        glm::vec3 calculateTangent(const glm::vec3& _normal, const glm::vec3& _bitangent);
		void saveVertex(const std::string& _type, const std::vector<RVertex*>& _v);
		void saveIndex(const std::string& _type, const std::vector<uint32_t>& _v);
		const std::vector<RVertex*>& getVertex(const std::string& _type);
		const std::vector<uint32_t>& getIndex(const std::string& _type);

        virtual void initTemplate() override;
    };

}
