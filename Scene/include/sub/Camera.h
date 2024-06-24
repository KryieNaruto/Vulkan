#pragma once
#include <Resource\include\RCamera.h>
#include "scene_obj.h"

using TextureEXT = ThirdParty::TextureEXT;

namespace Core::Resource {
	enum ENGINE_API_SCENE TEXTURE_TYPE
	{
		IMAGE = 0x00,
		COLOR_ATTACHMENT,
		DEPTH_ATTACHMENT,
	};

    class ENGINE_API_SCENE Camera :
		public SceneObject,
        public RCamera
    {
    public:
        Camera(const std::string& _name = "Camera");
        Camera(const VkExtent3D& _size, const std::string& _name = "Camera");
        virtual ~Camera();
        Camera* setMainCamera();
        bool& isMain() { return m_is_main; }
		virtual void propertyEditor() override;
        Camera* setClearValues(const std::vector<VkClearValue>& _values);
        TextureEXT* getCurrentTextureEXT(TEXTURE_TYPE _type);
        virtual void onUpdate();
        virtual void onInput();
		// 设置投影方式
		void setPorjectType(LIGHT_TYPE _tpye);
        Camera* setSubCamera(Camera* _sub) { p_sub_camear = _sub; return this; }
        Camera* getSubCamera() { return p_sub_camear; }
    protected:
        Camera* p_sub_camear = nullptr;
        std::unordered_map<TEXTURE_TYPE, std::vector<TextureEXT*>> p_attachments;
        bool m_is_main = false;

        void initTemplate() override;
    };

}

