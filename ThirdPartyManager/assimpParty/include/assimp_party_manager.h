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
        // 从文件中加载模型根节点数据，保存在p_root_scene
        void loadModel(const std::string& _path);
        // 获取根节点数据，进行加工,如果未被加载，则尝试加载
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

