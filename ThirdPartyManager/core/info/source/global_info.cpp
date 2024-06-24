#include "global_info.h"
#include <ThirdParty/SDL2/SDL.h>
#include <unordered_map>
namespace ThirdParty {
	namespace Core {
		ENGINE_API_THIRDPARTY ApplicationWindowInfo g_ApplicationWindowInfo(
			"Zs",
			// vk|显示|可变大小|高DPI|最大化
			SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED
		);

		ENGINE_API_THIRDPARTY ApplicationSDLInfo g_ApplicationSDLInfo;
		ENGINE_API_THIRDPARTY ApplicationVulkanInfo g_ApplicationVulkanInfo;
		ENGINE_API_THIRDPARTY ApplicationInfo g_ApplicationInfo(
#ifdef _DEBUG
			true, // m_debug
#else
			false,
#endif
			false
		);
		ENGINE_API_THIRDPARTY std::unordered_map<std::string, std::string> g_json_files;

		ENGINE_API_THIRDPARTY std::vector<void*> g_vk_objects;
		// key : path
		ENGINE_API_THIRDPARTY std::unordered_map<std::string, void*> g_textureEXT_loaded;


		ENGINE_API_THIRDPARTY ApplicationInfo& appInfo = ThirdParty::Core::g_ApplicationInfo;
		ENGINE_API_THIRDPARTY ApplicationWindowInfo& windowInfo = ThirdParty::Core::g_ApplicationWindowInfo;
		ENGINE_API_THIRDPARTY ApplicationSDLInfo& sdlInfo = ThirdParty::Core::g_ApplicationSDLInfo;
		ENGINE_API_THIRDPARTY ApplicationVulkanInfo& vkInfo = ThirdParty::Core::g_ApplicationVulkanInfo;
	}
}