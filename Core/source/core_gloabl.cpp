#include "core_global.h"
#include <ThirdPartyManager/third_party_manager_global.h>
#include "ZClass.h"

namespace Core {
	// ������Դ
	ENGINE_API_CORE std::map<uint64_t, void*> Core_Pool::p_all_resources;
#ifdef _DEBUG
	std::map<void*, Core_Pool::ptr_debug_info*> Core_Pool::p_all_resources_debug;

#endif // DEBUG
	// ���д���
	ENGINE_API_CORE std::map<uint64_t, void*> Core_Pool::p_all_editor;
	// RenderPass
	ENGINE_API_CORE std::map<std::string, void*> Core_Pool::g_all_renderpass;
	// ThirdParty::Pipeline
	ENGINE_API_CORE std::map<std::string, void*> Core_Pool::g_all_pipelines;
	// ThirdParty::Sampler
	ENGINE_API_CORE std::map<std::string, void*> Core_Pool::g_all_samplers;

	ENGINE_API_CORE void Core_Pool::addResource_debug(uint64_t _uuid,void* _res, size_t _line, const char* _file)
	{
		Core_Pool::p_all_resources[_uuid] = _res;
#ifdef _DEBUG
		Core_Pool::ptr_debug_info* _info = new Core_Pool::ptr_debug_info;
		_info->m_file = _file;
		_info->m_line = _line;
		_info->m_uuid = _uuid;
		p_all_resources_debug[_res] = _info;
#endif
	}


	ENGINE_API_CORE void Core_Pool::removeResource_debug(uint64_t _uuid, void* _res)
	{
#ifdef _DEBUG
		auto _info = p_all_resources_debug[_res];
		if (_info) {
			Core_Pool::p_all_resources.erase(_info->m_uuid);
			Core_Pool::p_all_resources_debug.erase(_res);
			delete _info;
		}
		else {
			Core_Pool::p_all_resources.erase(_uuid);
		}
#else
		Core_Pool::p_all_resources.erase(_uuid);
#endif
	}


	namespace Core_Pool {
		ENGINE_API_CORE void destroyEditor() {
			for (const auto& editor : p_all_editor) {
				p_all_resources.erase(editor.first);
				delete (ZClass*)editor.second;
			}
		}

		ENGINE_API_CORE void destroyAllResource()
		{
			for (const auto& [_uuid,_res]:p_all_resources) {
				delete (ZClass*)_res;
#ifdef _DEBUG
				auto _info = p_all_resources_debug[_res];
				p_all_resources_debug.erase(_res);
				delete _info;
#endif // _DEBUG
			}
		}
	}

	extern ENGINE_API_CORE ProjectInfo* g_project_info = nullptr;
	extern ENGINE_API_CORE std::string g_old_file_path = "";
	extern ENGINE_API_CORE ZFileManager* g_file_manager = new ZFileManager();

	ENGINE_API_CORE void destroy() {
		// ɾ��project��Ϣ
		if (g_project_info) delete g_project_info;
		// �������б༭��
		Core_Pool::destroyEditor();
		// ����������Դ
		Core_Pool::destroyAllResource();
		// �����ļ���Դ������
		delete g_file_manager;
	}

	void save()
	{
		// ������Ŀ����
		g_project_info->save();
	}

}