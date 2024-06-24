#pragma once
#ifndef GLOBAL_INFO_H
#define GLOBAL_INFO_H

#include "core/info/include/application_window_info.h"
#include <unordered_map>

namespace ThirdParty {
	namespace Core {
		ENGINE_API_THIRDPARTY extern ApplicationInfo g_ApplicationInfo;
		ENGINE_API_THIRDPARTY extern ApplicationWindowInfo g_ApplicationWindowInfo;
		ENGINE_API_THIRDPARTY extern ApplicationSDLInfo g_ApplicationSDLInfo;
		ENGINE_API_THIRDPARTY extern ApplicationVulkanInfo g_ApplicationVulkanInfo;
		/**
		 * 文件读取json,保存为总的json字符串
		 * param 0: json file path
		 * param 1: json file string
		 */
        ENGINE_API_THIRDPARTY extern std::unordered_map<std::string,std::string> g_json_files;

		/**
		 * vkObjects
		 */
		ENGINE_API_THIRDPARTY extern std::vector<void*> g_vk_objects;
		ENGINE_API_THIRDPARTY extern std::unordered_map<std::string, void*> g_textureEXT_loaded;

		ENGINE_API_THIRDPARTY extern ApplicationInfo& appInfo;
		ENGINE_API_THIRDPARTY extern ApplicationWindowInfo& windowInfo;
		ENGINE_API_THIRDPARTY extern ApplicationSDLInfo& sdlInfo;
		ENGINE_API_THIRDPARTY extern ApplicationVulkanInfo& vkInfo;
	}
}

#endif // GLOBAL_INFO_H