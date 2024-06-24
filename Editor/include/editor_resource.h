#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <ThirdPartyManager/third_party_manager_global.h>


#define BUTTON_PNG_PREVIOUS		"��1_left-one.png"
#define BUTTON_PNG_NEXT			"��1_right-one.png"
#define BUTTON_PNG_UP			"��1_up-one.png"
#define ICON_FOLDER				"�ļ���.png"
#define ICON_FILE				"�ļ�.png"
#define ICON_MATERIAL			"����_material-two.png"
#define ICON_MESH				"BIMӦ��.png"
#define ICON_MODEL				"��ģ��.png"
#define ICON_FILE_MODEL			"����ģ���ļ�.png"
#define ICON_FILE_IMAGE			"ͼƬ.png"
#define ICON_FILE_JSON			"JSON�ļ�.png"
#define ICON_FILE_BINARY		"�����ļ�.png"
#define ICON_FILE_TEXT			"txt.png"
#define ICON_FILE_SHADER		"��ɫ��.png"

namespace Editor {
	// ͼ����Դ
	extern std::vector<ThirdParty::TextureEXT*>				p_textures;
	extern std::unordered_map<std::string, VkDescriptorSet> p_icons;

	// ��ȡͼ��ID
	inline VkDescriptorSet getIcon(const std::string& _name) { return p_icons[_name]; }
}
