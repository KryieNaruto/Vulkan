#include "third_party_manager_global.h"

namespace ThirdParty {
	ENGINE_API_THIRDPARTY SDLManager& sdl_manager = ThirdParty::sdlManager;
	ENGINE_API_THIRDPARTY VkManager& vk_manager = ThirdParty::vkManager;
	ENGINE_API_THIRDPARTY AssimpManager& assimp_manager = ThirdParty::assimpManager;
	ENGINE_API_THIRDPARTY ImGuiManager imgui_manager;
	ENGINE_API_THIRDPARTY JsoncppManager jsoncpp_manager;
	ENGINE_API_THIRDPARTY ShaderCompiler* shader_compiler;


	bool isReady()
	{
		return (sdl_manager.isReady() && vk_manager.isReady() && imgui_manager.isReady() && jsoncpp_manager.isReady());
	}

	void init()
	{
		ThirdParty::sdl_manager.init();
		ThirdParty::vk_manager.init();
		ThirdParty::imgui_manager.init();
		ThirdParty::jsoncpp_manager.init();
		ThirdParty::assimp_manager.init();
	}

	void destroy()
	{
		if (shader_compiler) {
			delete shader_compiler;
			shader_compiler = nullptr;
		}
		auto& _a = ThirdParty::Core::g_vk_objects;
		for (size_t i = 0; i < _a.size();i++) {
			vkObject* _ptr = (vkObject*)_a[i];
			delete _ptr;
			_a[i] = nullptr;
		}
		ThirdParty::sdl_manager.destroy();
		ThirdParty::imgui_manager.destroy();
		ThirdParty::vk_manager.destroy();
		ThirdParty::jsoncpp_manager.destroy();
		ThirdParty::assimp_manager.destroy();

#ifdef _DEBUG
		if (allocList) {
			Core::warn("Î´ÊÍ·ÅÊýÁ¿:%d", allocList->size());
			for (const auto& _info : *allocList) {
				auto& _i = _info.second;
				Core::info("Address:%p", _i->address);
				Core::info("Size:%d", _i->size);
				Core::info("File:%s:%d", _i->file, _i->line);
				Core::warn("--------------------------");
				delete _i;
			}
		}
#endif // DEBUG

	}

}