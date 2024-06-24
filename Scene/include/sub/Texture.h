#pragma once
#include <Resource\include\RTexture.h>
#include <set>
#include "scene_obj.h"

namespace Core::Resource {

    //-----------------------------------------
    // Scene �µ�Texture
    //-----------------------------------------

    class Texture :
		public SceneObject,
        public RTexture
    {
    public:
        Texture() = default;
        // �ļ�����ͼ��
        Texture(const std::string& _path);
        // ��Ϊͼ�����, ��ͼ��
        Texture(uint32_t _width, uint32_t _height);
        virtual ~Texture();
		virtual void propertyEditor() override;

    private:
        ThirdParty::TextureEXT* p_texture_ext;
    };

}

