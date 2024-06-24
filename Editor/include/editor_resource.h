#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <ThirdPartyManager/third_party_manager_global.h>


#define BUTTON_PNG_PREVIOUS		"左1_left-one.png"
#define BUTTON_PNG_NEXT			"右1_right-one.png"
#define BUTTON_PNG_UP			"上1_up-one.png"
#define ICON_FOLDER				"文件夹.png"
#define ICON_FILE				"文件.png"
#define ICON_MATERIAL			"材质_material-two.png"
#define ICON_MESH				"BIM应用.png"
#define ICON_MODEL				"简单模型.png"
#define ICON_FILE_MODEL			"导入模型文件.png"
#define ICON_FILE_IMAGE			"图片.png"
#define ICON_FILE_JSON			"JSON文件.png"
#define ICON_FILE_BINARY		"数据文件.png"
#define ICON_FILE_TEXT			"txt.png"
#define ICON_FILE_SHADER		"着色器.png"

namespace Editor {
	// 图标资源
	extern std::vector<ThirdParty::TextureEXT*>				p_textures;
	extern std::unordered_map<std::string, VkDescriptorSet> p_icons;

	// 获取图标ID
	inline VkDescriptorSet getIcon(const std::string& _name) { return p_icons[_name]; }
}
