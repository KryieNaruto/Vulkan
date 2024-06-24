#pragma once
#include <map>
#include "ZFileManager.h"
#include <unordered_map>
#include <format>
#include <iostream>

#define LOG(_color, ...) \
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE); \
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | _color);	\
	std::cout<<__FILE__<<":"<<__LINE__<<" : ";\
	std::cout<<std::format(__VA_ARGS__)<<std::endl;	\
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);

#define LOG_INFO(...)		LOG(8  ,__VA_ARGS__);
#define LOG_SUCCESS(...)	LOG(2  ,__VA_ARGS__);
#define LOG_WARN(...)		LOG(6  ,__VA_ARGS__);
#define LOG_DEBUG(...)		LOG(11 ,__VA_ARGS__);
#define LOG_ERROR(...)		LOG(4  ,__VA_ARGS__);
//=====================
// ȫ����Դ�����
//=====================
namespace Core {
	namespace Core_Pool {
		// ������Դ
		extern __declspec(dllimport) std::map<uint64_t, void*> p_all_resources;
#ifdef _DEBUG
		struct ENGINE_API_CORE ptr_debug_info {
			size_t m_line;
			std::string m_file;
			uint64_t m_uuid;
		};
		extern std::map<void*, ptr_debug_info*> p_all_resources_debug;
#endif // DEBUG
		// ���д���(��Ⱦ�ɼ�)
		extern __declspec(dllimport) std::map<uint64_t, void*> p_all_editor;
		// ��ˮ��
		extern __declspec(dllimport) std::map<std::string, void*> g_all_renderpass;
		// ThirdParty::Pipeline
		extern __declspec(dllimport) std::map<std::string, void*> g_all_pipelines;
		// ThirdParty::Sampler
		extern __declspec(dllimport) std::map<std::string, void*> g_all_samplers;

		ENGINE_API_CORE void destroyEditor();
		ENGINE_API_CORE void destroyAllResource();

		ENGINE_API_CORE void addResource_debug(uint64_t _uuid, void* _res, size_t _line,const char* _file);
		ENGINE_API_CORE void removeResource_debug(uint64_t _uuid, void* _res = nullptr);

	};

	// ��Ŀ��Ϣ
	extern __declspec(dllimport) ProjectInfo* g_project_info;
	// ���ļ�·��
	extern __declspec(dllimport) std::string g_old_file_path;

	// �ļ�����
	extern __declspec(dllimport) ZFileManager* g_file_manager;

	ENGINE_API_CORE void destroy();

	// ������Ŀ
	ENGINE_API_CORE void save();

}

#define addResource(uuid,res)		addResource_debug(uuid, res, __LINE__, __FILE__)
#define removeResource(uuid)		removeResource_debug(uuid, nullptr)
#define removeResource(uuid,res)	removeResource_debug(uuid, res)
