#include "imgui_utils.h"
#include "core/common/include/debug_to_console.h"
#include "sdl_party_manager.h"

ENGINE_API_THIRDPARTY float ThirdParty::GetTabHeight(const char* title)
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 textSize = ImGui::CalcTextSize(title);
	ImVec2 padding = style.FramePadding;
	float borderSize = style.FrameBorderSize;

	return textSize.y + (padding.y * 2) + borderSize;
};

namespace ThirdParty::imgui {

	ENGINE_API_THIRDPARTY void help(const std::string& _path)
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text,IM_COL32(150,150,150,255));
		ImGui::Text("(?)");
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered()) {
			if (ImGui::BeginTooltip()) {
				ImGui::Text(string_To_UTF8(_path).c_str());
				ImGui::EndTooltip();
			}
		};
	}
}