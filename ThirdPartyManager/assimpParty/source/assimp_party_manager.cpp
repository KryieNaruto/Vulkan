#include "assimp_party_manager.h"
#include "core/common/include/debug_to_console.h"
#include <ThirdParty/assimp/postprocess.h>
#include <filesystem>
#include <unordered_map>


std::unordered_map<std::string, const aiScene*> ThirdParty::AssimpManager::m_model_aiScenes;

ENGINE_API_THIRDPARTY ThirdParty::AssimpManager ThirdParty::assimpManager;

void ThirdParty::AssimpManager::loadModel(const std::string& _path)
{
	std::filesystem::path path(_path);
	if (!std::filesystem::exists(path) || std::filesystem::is_directory(_path)) {
		Core::error("Failed to load Model:%s", _path.c_str());
		return;
	}
	// TODO... 多线程
	p_root_scene = m_importer.ReadFile(_path,
		aiProcess_ConvertToLeftHanded |
		aiProcess_FixInfacingNormals |
		aiProcess_Triangulate |
		aiProcess_FixInfacingNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType
	);
	if (!p_root_scene) {
		Core::error("Failed to load Model:%s.", _path.c_str());
		Core::error("ASSIMP:%s", m_importer.GetErrorString());
		return;
	}
	if (p_root_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !p_root_scene->mRootNode) {
		Core::error("Failed to load Model:%s.");
		Core::error("ASSIMP::%s", m_importer.GetErrorString());
		return;
	}
	m_model_aiScenes[std::filesystem::absolute(path).string()] = p_root_scene;
}

const aiScene* ThirdParty::AssimpManager::getModel(const std::string& _path)
{
	const auto& path = std::filesystem::absolute(_path);
	const auto& _rel = m_model_aiScenes.find(path.string());
	// 如果未被加载，则加载
	if (_rel == m_model_aiScenes.end()) {
		loadModel(_path);
		return m_model_aiScenes[path.string()];
	}
	return _rel->second;
}

ThirdParty::AssimpManager::~AssimpManager()
{

}

ThirdParty::AssimpManager* ThirdParty::AssimpManager::init()
{
	this->m_isInitialized = true;
	return this;
}

void ThirdParty::AssimpManager::destroy()
{

}

void ThirdParty::AssimpManager::windowResize()
{

}
