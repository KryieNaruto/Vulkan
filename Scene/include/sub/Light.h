#pragma once
#include <Resource\include\RLight.h>
#include "scene_obj.h"
#include "shape/Shape.h"
namespace Core::Resource {

	class ENGINE_API_SCENE Light :
        public SceneObject,
		public RLight
    {
    public:
        Light(const std::string& _name = "Light");
        Light(LIGHT_TYPE _type, const glm::vec4& _light_color, const std::string& _name = "Light");
        Light(LIGHT_TYPE _type,const glm::vec3& _dir, const glm::vec4& _light_color, const std::string& _name = "Light");
        Light(LIGHT_TYPE _type,const glm::vec3& _pos, const glm::vec3& _dir, const glm::vec4& _light_color, const std::string& _name = "Light");
        virtual ~Light();
        RMesh* getMesh();
        virtual Shader* getShader() { return p_shape->getShader(); }
        void draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func) override;
		virtual void propertyEditor() override;
        Light* update();
        Light* setShader(Shader* _shader);
        bool& isPosChange() { return m_is_pos_change; }
    private:
        bool m_is_pos_change = true;
        Shape* p_shape = nullptr;
        // 用于输出深度图
        void* p_sub_camera = nullptr;
    public:
        inline Light* setShape(Shape* _p_shape) { p_shape = _p_shape; return this; }
        inline Shape* getShape() { return p_shape; }
		inline LIGHT_TYPE getLightType() {
            return p_light_property->m_type;
		}
        inline void setRenderCamera(void* _cam) { p_sub_camera = _cam; }
        inline void* getRenderCamera() { return p_sub_camera; }
	};

}

