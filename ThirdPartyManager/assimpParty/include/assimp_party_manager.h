#pragma once
#include "common/include/third_party_manager.h"
#include "core/info/include/global_info.h"
#include <ThirdParty/assimp/Importer.hpp>
#include <ThirdParty/assimp/scene.h>
#include <unordered_map>
#include <string>

namespace ThirdParty {
    class ENGINE_API_THIRDPARTY AssimpManager:
        public Manager<AssimpManager>
    {
    public:
        // ���ļ��м���ģ�͸��ڵ����ݣ�������p_root_scene
        void loadModel(const std::string& _path);
        // ��ȡ���ڵ����ݣ����мӹ�,���δ�����أ����Լ���
        const aiScene* getModel(const std::string& _path);
    public:
        AssimpManager() = default;
        virtual ~AssimpManager();
		virtual AssimpManager* init() override;
		virtual void destroy() override;
		virtual void windowResize() override;
    protected:
    private:
        static std::unordered_map<std::string,const aiScene*> m_model_aiScenes;
        Assimp::Importer m_importer;
        const aiScene* p_root_scene = nullptr;

    };
	extern ENGINE_API_THIRDPARTY AssimpManager assimpManager;
}

