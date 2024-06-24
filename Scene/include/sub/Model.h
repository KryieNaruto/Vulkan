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
		inline static std::set<Model*> p_models; // 所有加载过的model
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
        // draw_func 与Model 结合
		virtual void draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func) override;
        // draw_func 与Model 分离，与Shader结合, _draw_func 在外部执行，mat在外部绑定
		virtual void draw(VkCommandBuffer _cmd);
        const std::vector<Core::Resource::Material*>& getMaterials();
        const std::unordered_map<uint32_t, std::vector<Core::Resource::Mesh*>> getMeshes();
        Mesh* getMesh(uint32_t _index);
        // 根据Shder创建Material,并保存，以Shader为索引，进行快速切换shader
        // _temp 表示是否是临时Shader, 生效一次DrawCall后切换回原来的shader
        // 切换Material组
        virtual Model* setShader(Shader* _shader, bool _temp = false);
        // 设置渲染相机，其会复制一份当前Material组以应对多个摄像共用一份Material的问题
        virtual Model* setRenderCamera(void* _cam);
        // 设置临时Material, 只生效一次
        virtual Model* setMaterial(Material* _material);
        // 设置材质, 仅限mesh数量为1.多mesh会造成Texture分配问题。
        virtual Model* setCurrentMaterial(Material* _material);
        bool isReady();
        inline std::unordered_map<void*, std::unordered_map<Shader*, std::vector<Material*>>>& getMaterialsCopyCameraUse() { return p_materials_copy_camera_use; }
    protected:
        std::set<Shader*> p_shader_loaded;
        std::unordered_map<void*, std::unordered_map<Shader*, std::vector<Material*>>> p_materials_copy_camera_use;     // 不同摄像下的shader与mat的映射关系,用于加快材质切换
        std::unordered_map<Shader*, std::vector<Material*>> p_materials_copy_loaded;
        std::vector<Material*> p_materials_copy;    // RMaterial 转Material
        std::unordered_map<uint32_t, std::vector<Mesh*>> p_merge_meshes;
        std::vector<Mesh*> p_merge_meshes_vec;
        Mesh* p_mesh_no_texture;
        // mesh合并，相同Material Index的mesh合并，以优化性能
        bool m_merge_finish = false;
        void merge();

        // 临时使用Material
        bool m_temp_draw_in_material = false;
        ThirdParty::VertexBuffer* p_vertex_buffer_last = nullptr;
        Shader* p_shader_last = nullptr;
        void* p_render_camera = nullptr;
	};

}

