#pragma once
#include "core/info/include/global_info.h"
#include "core/info/include/application_window_info.h"
#include "sdlParty/include/sdl_party_manager.h"
#include "vkParty/include/vk_party_manager.h"
#include "imguiParty/include/imgui_party_manager.h"
#include "imguiParty/include/imgui_material_editor.h"
#include "assimpParty/include/assimp_party_manager.h"
#include "jsoncppParty/include/jsoncpp_party_manager.h"
#include "vkParty/include/vk_util_function.h"
#include "vkParty/include/vkobject/vk_command_buffer.h"
#include "vkParty/include/vkobject/vk_queue.h"
#include "vkParty/include/vkobject/vk_render_pass.h"
#include "vkParty/include/vkobject/vk_pipeline_layout.h"
#include "vkParty/include/vkobject/vk_pipeline.h"
#include "vkParty/include/vkobject/vk_texture.h"
#include "vkParty/include/vkobject/vk_image.h"
#include "vkParty/include/vkobject/vk_frame_buffer.h"
#include "vkParty/include/vkobject/vk_fence.h"
#include "vkParty/include/vkobject/vk_semaphore.h"
#include "vkParty/include/vkobject/vk_subpass.h"
#include "vkParty/include/vkobject/vk_shader_compiler.h"
#include "vkParty/include/vkobject/vk_image_view.h"
#include "vkParty/include/vkobject/vk_sampler.h"
#include "vkParty/include/vkobject/vk_buffer.h"
#include "vkParty/include/vkobject/vk_vertex_buffer.h"
#include "vkParty/include/vkobject/vk_index_buffer.h"
#include "vkParty/include/vkobject/vk_uniform_buffer.h"
#include "vkParty/include/vkobject/vk_descriptor_set.h"
#include "vkParty/include/vkobject/vk_texture_ext.h"
#include "vkParty/include/vkobject/vk_slot.h"

namespace ThirdParty {
	ENGINE_API_THIRDPARTY extern SDLManager& sdl_manager;
	ENGINE_API_THIRDPARTY extern VkManager& vk_manager;
	ENGINE_API_THIRDPARTY extern AssimpManager& assimp_manager;
	ENGINE_API_THIRDPARTY extern ImGuiManager imgui_manager;
	ENGINE_API_THIRDPARTY extern JsoncppManager jsoncpp_manager;
	ENGINE_API_THIRDPARTY extern ShaderCompiler* shader_compiler;

	
	ENGINE_API_THIRDPARTY extern ThirdParty::Core::ApplicationInfo g_ApplicationInfo;
	ENGINE_API_THIRDPARTY extern ThirdParty::Core::ApplicationWindowInfo g_ApplicationWindowInfo;
	ENGINE_API_THIRDPARTY extern ThirdParty::Core::ApplicationSDLInfo g_ApplicationSDLInfo;
	ENGINE_API_THIRDPARTY extern ThirdParty::Core::ApplicationVulkanInfo g_ApplicationVulkanInfo;

	ENGINE_API_THIRDPARTY bool isReady();
	ENGINE_API_THIRDPARTY void init();
	ENGINE_API_THIRDPARTY void destroy();
}