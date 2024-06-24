#pragma once
#include "ZResource.h"

namespace Core::Resource {
    
    class ENGINE_API_RESOURCE RTexture :
        public ZResource
    {
    public:
        RTexture(const std::string& _name = "RTexture");
        RTexture(const std::string& _path,aiTextureType _type, const std::string& _name = "RTexture");
		virtual ~RTexture();
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& _root) override;

        // �������ģ��·��
        inline const std::string& getPath() {
            return m_path;
        }
    protected:
        std::string m_path;
        RESOURCE_TYPE m_type;
        RESOURCE_TYPE aiTextureTypeToResourceType(aiTextureType _type);

        // ͨ�� ZResource �̳�
        void initTemplate() override;
    };

}

