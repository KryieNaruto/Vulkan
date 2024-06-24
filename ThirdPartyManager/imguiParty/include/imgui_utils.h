#pragma once
#include <ThirdParty/imgui/imgui.h>
#include "core/info/include/application_window_info.h"
#include "common/include/third_party_manager.h"
namespace ThirdParty {
	ENGINE_API_THIRDPARTY float GetTabHeight(const char* title);

	// 组件
	namespace imgui {
		// （？）小组件
		ENGINE_API_THIRDPARTY void help(const std::string& _path);
	}
}