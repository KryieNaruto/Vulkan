#pragma once
#include <Resource/resource_global.h>
#include "scene_obj.h"
namespace Core::Resource {
    
    class ENGINE_API_SCENE Mesh :
        public SceneObject,
        public RMesh
    {
    public:
        Mesh(const std::vector<RMesh*>& _mesh);
        virtual ~Mesh();
        void propertyEditor() override;
        virtual void draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func);
        virtual void draw(VkCommandBuffer _cmd);
        inline void* getParent() { return p_parent_model; }
        inline void setParent(void* _model) { p_parent_model = _model; }
    private:
        void* p_parent_model = nullptr;
    };
}

