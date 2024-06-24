#include "vkobject/vk_render_pass.h"
#include <core/common/include/debug_to_console.h>
ThirdParty::RenderPass::RenderPass(VkRenderPass _renderpass, const std::string& _label)
	:vkObject(_label),m_render_pass(_renderpass)
{
	if (!this->p_begin_info) {
		this->p_begin_info = new VkRenderPassBeginInfo;
		this->p_begin_info->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		this->p_begin_info->pNext = nullptr;
	}
}

ThirdParty::RenderPass::~RenderPass()
{
	if (this->p_begin_info) delete this->p_begin_info;
	auto& _a = ThirdParty::Core::g_vk_objects;
	for (auto ite = _a.begin(); ite < _a.end(); ite++)
	{
		if (*ite == this) {
			_a.erase(ite);
			break;
		}
	}
}

void ThirdParty::RenderPass::beginRenderPass(VkCommandBuffer _commandBuffer, VkSubpassContents _contents)
{
	bool available = true;
	if (m_render_pass == VK_NULL_HANDLE) { Core::warn("[Render Pass] Render Pass has not been set."); available &= false; }
	if (!m_flag[(int)RENDERPASS_BEGIN_FLAG::CLEAR_VALUE]) { Core::warn("[Render Pass] Clear value has not been set."); available &= false; }
	if (!m_flag[(int)RENDERPASS_BEGIN_FLAG::FRAME_BUFFER]) { Core::warn("[Render Pass] Framebuffer has not been set."); available &= false; }
	if (!m_flag[(int)RENDERPASS_BEGIN_FLAG::RENDER_AREA]) { Core::warn("[Render Pass] Render area has not been set."); available &= false; }
	if (!available) return;
	
	// RENDER PASS
	p_begin_info->renderPass = m_render_pass;
	// FRAME BUFFER
	p_begin_info->framebuffer = t_framebuffer;
	// RENDER AREA
	p_begin_info->renderArea = t_render_area;
	// CLEAR VALUES
	p_begin_info->clearValueCount = static_cast<uint32_t>(t_clear_values.size());
	p_begin_info->pClearValues = t_clear_values.data();

	vkCmdBeginRenderPass(_commandBuffer, p_begin_info, _contents);
	t_commandBuffer = _commandBuffer;
}

void ThirdParty::RenderPass::endRenderPass()
{
	vkCmdEndRenderPass(t_commandBuffer);
}
