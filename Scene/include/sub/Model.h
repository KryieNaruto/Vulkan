#pragma once
#include <Resource\include\RModel.h>
#include "scene_obj.h"
#include "Material.h"
#include "Mesh.h"
#include <unordered_map>

namespace Core::Resource {

    class ENGINE_API_SCENE Model :
        public SceneObject,
        public RModel
    {
    public:
		inline static std::set<Model*> p_models; // ���м��ع���model
        static void addModel(Model* _m) { p_models.insert(_m); }
        static void removeModel(Model* _m) { 
            p_models.erase(_m); 
            Core::Core_Pool::removeResource(_m->getUUID(), _m);
        }

        Model();
        Model(const std::string& _path,const std::string& _name = "Model");
        virtual ~Model();

        RMesh* getRMesh(uint32_t _index) { return getRMeshes()[_index]; }
		virtual void propertyEditor() override;
        // draw_func ��Model ���
		virtual void draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func) override;
        // draw_func ��Model ���룬��Shader���, _draw_func ���ⲿִ�У�mat���ⲿ��
		virtual void draw(VkCommandBuffer _cmd);
        const std::vector<Core::Resource::Material*>& getMaterials();
        const std::unordered_map<uint32_t, std::vector<Core::Resource::Mesh*>> getMeshes();
        Mesh* getMesh(uint32_t _index);
        // ����Shder����Material,�����棬��ShaderΪ���������п����л�shader
        // _temp ��ʾ�Ƿ�����ʱShader, ��Чһ��DrawCall���л���ԭ����shader
        // �л�Material��
        virtual Model* setShader(Shader* _shader, bool _temp = false);
        // ������Ⱦ�������Ḵ��һ�ݵ�ǰMaterial����Ӧ�Զ��������һ��Material������
        virtual Model* setRenderCamera(void* _cam);
        // ������ʱMaterial, ֻ��Чһ��
        virtual Model* setMaterial(Material* _material);
        // ���ò���, ����mesh����Ϊ1.��mesh�����Texture�������⡣
        virtual Model* setCurrentMaterial(Material* _material);
        bool isReady();
        inline std::unordered_map<void*, std::unordered_map<Shader*, std::vector<Material*>>>& getMaterialsCopyCameraUse() { return p_materials_copy_camera_use; }
    protected:
        std::set<Shader*> p_shader_loaded;
        std::unordered_map<void*, std::unordered_map<Shader*, std::vector<Material*>>> p_materials_copy_camera_use;     // ��ͬ�����µ�shader��mat��ӳ���ϵ,���ڼӿ�����л�
        std::unordered_map<Shader*, std::vector<Material*>> p_materials_copy_loaded;
        std::vector<Material*> p_materials_copy;    // RMaterial תMaterial
        std::unordered_map<uint32_t, std::vector<Mesh*>> p_merge_meshes;
        std::vector<Mesh*> p_merge_meshes_vec;
        Mesh* p_mesh_no_texture;
        // mesh�ϲ�����ͬMaterial Index��mesh�ϲ������Ż�����
        bool m_merge_finish = false;
        void merge();

        // ��ʱʹ��Material
        bool m_temp_draw_in_material = false;
        ThirdParty::VertexBuffer* p_vertex_buffer_last = nullptr;
        Shader* p_shader_last = nullptr;
        void* p_render_camera = nullptr;
	};

}

