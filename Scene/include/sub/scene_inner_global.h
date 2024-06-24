#pragma once
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <functional>

#ifdef SCENE_EXPORTS
#define ENGINE_API_SCENE __declspec(dllexport)
#else
#define ENGINE_API_SCENE __declspec(dllimport)
#endif

#define SHADER_DATA_BINDING_FUNC_LAMBDA_WITH_ARGS(...)		\
[__VA_ARGS__](void* _scene, void* _mat, void* _scene_obj,int _mesh_index, void* _mesh = nullptr, void* _camera = nullptr)->void

#define SHADER_DATA_BINDING_FUNC_LAMBDA		\
[](void* _scene, void* _mat, void* _scene_obj,int _mesh_index, void* _mesh = nullptr, void* _camera = nullptr)->void

// 数据绑定函数
// Core::Resource::Scene*
// Core::Resource::Material*
// Core::Resource::Model*
// mesh Index
// Core::Resource::Mesh	_merge_no_texture
using Shader_Data_Binding_Func_Type = void(void*, void*, void*, int, void*, void*);
// ---------------------------
// 数据绑定函数
// Core::Resource::Scene*
// Core::Resource::Material*
// Core::Resource::Model*
// mesh Index
// Core::Resource::Mesh	_merge_no_texture
using Shader_Data_Binding_Func = std::function<Shader_Data_Binding_Func_Type>;

namespace Core::Resource {
	inline static std::unordered_set<std::string> g_names;    // 所有存在过的名称[Scene中]
	inline static std::unordered_map<std::string, void*> p_loaded_models;
	// 获取可用名称
	inline std::string getName(const std::string& _name);
	inline void* getModel(const std::string& _path);

}

std::string Core::Resource::getName(const std::string& _name)
{
	auto name = _name;
	uint32_t i = 0;
	while (g_names.find(name) != g_names.end()) {
		name = _name + " " + std::to_string(i++);
	}
	g_names.insert(name);
	return name;
}

void* Core::Resource::getModel(const std::string& _path)
{
	auto _ite = p_loaded_models.find(_path);
	if (_ite != p_loaded_models.end())
		return _ite->second;
	else return nullptr;
}
