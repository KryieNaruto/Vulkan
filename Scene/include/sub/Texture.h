#pragma once
#include <Resource\include\RTexture.h>
#include <set>
#include "scene_obj.h"

namespace Core::Resource {

    //-----------------------------------------
    // Scene 下的Texture
    //-----------------------------------------

    class Texture :
		public SceneObject,
        public RTexture
    {
    public:
        Texture() = default;
        // 文件加载图像
        Texture(const std::string& _path);
        // 作为图像接收, 空图像
        Texture(uint32_t _width, uint32_t _height);
        virtual ~Texture();
		virtual void propertyEditor() override;

    private:
        ThirdParty::TextureEXT* p_texture_ext;
    };

}

