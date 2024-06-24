#include "vkobject/vk_subpass.h"

ThirdParty::Subpass::Subpass(const std::string& _label)
	:vkObject(_label)
{
	if (!p_description) {
		p_description = new VkSubpassDescription;
		p_description->flags = 0;
		p_description->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		p_description->inputAttachmentCount = 0;
		p_description->pInputAttachments = 0;
		p_description->colorAttachmentCount = 0;
		p_description->pColorAttachments = 0;
		p_description->pResolveAttachments = 0;
		p_description->pDepthStencilAttachment = 0;
		p_description->preserveAttachmentCount = 0;
		p_description->pPreserveAttachments = 0;

	}
}

ThirdParty::Subpass::~Subpass()
{
	if (p_description) delete p_description;
}

VkSubpassDescription& ThirdParty::Subpass::getVkSubpassDescription()
{
	{
		p_description->inputAttachmentCount = m_attachment_input.size();
		p_description->pInputAttachments = m_attachment_input.data();
		p_description->colorAttachmentCount = m_attachment_color.size();
		p_description->pColorAttachments = m_attachment_color.data();
		p_description->pResolveAttachments = m_attachment_resolve.data();
		p_description->pDepthStencilAttachment = m_attachment_depth_stencil.data();
	}

	return *p_description;
}

ThirdParty::Subpass* ThirdParty::Subpass::addColorAttachmentRef(uint32_t _att_index, VkImageLayout _layout)
{
	VkAttachmentReference _a = { _att_index,_layout };
	m_attachment_color.push_back(_a);
	return this;

}

ThirdParty::Subpass* ThirdParty::Subpass::addInputAttachmentRef(uint32_t _att_index, VkImageLayout _layout)
{
	VkAttachmentReference _a = { _att_index,_layout };
	m_attachment_input.push_back(_a);
	return this;
}

ThirdParty::Subpass* ThirdParty::Subpass::addResolveAttachmentRef(uint32_t _att_index, VkImageLayout _layout)
{
	VkAttachmentReference _a = { _att_index,_layout };
	m_attachment_resolve.push_back(_a);
	return this;
}

ThirdParty::Subpass* ThirdParty::Subpass::addDepthStencilAttachmentRef(uint32_t _att_index, VkImageLayout _layout)
{
	VkAttachmentReference _a = { _att_index,_layout };
	m_attachment_depth_stencil.push_back(_a);
	return this;
}
