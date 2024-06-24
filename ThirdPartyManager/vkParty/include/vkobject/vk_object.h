#pragma once
#include <string>
#include <ThirdParty/vulkan/vulkan.h>
#include <core/info/include/global_info.h>
#include "core/common/include/debug_to_console.h"
#include "imguiParty/include/imgui_material_editor.h"
#include "vkParty/include/vk_util_function.h"
namespace ThirdParty {

	/************************************************************************/
	/* vk����ͳһ����
	���vkObject���캯������_label,��ᱣ�浽ȫ��ͳһ�ͷţ�������Ҫ�ֶ��ͷ� */
	/************************************************************************/

	class ENGINE_API_THIRDPARTY vkObject
	{
	public:
		vkObject() = default;
		vkObject(const std::string& _label);
		virtual ~vkObject();
		static void removeVkObject(vkObject* _obj);
		static void addVkObject(vkObject* _obj);
		void removeFromVkObjects();
		imgui::Template* getTemplate() {return p_template;}
		inline const std::string& getLabel() {return m_label;}
		virtual void initObjFromTemplate();
	protected:
		virtual void initTemplate();

		std::string m_label = "vkObject";
		VkResult m_result;
		ThirdParty::Core::ApplicationVulkanInfo& m_vkInfo = ThirdParty::Core::vkInfo;

		bool m_into_pool = false;
		imgui::Template* p_template = nullptr;
	};

}

