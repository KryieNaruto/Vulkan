#include "RRenderPass.h"
#include <Core/include/core_global.h>

Core::Resource::RRenderPass::RRenderPass()
{
	this->m_res_name = "";
	this->m_json_title = "RRenderPass";
	this->p_id_generate->init(RESOURCE_TYPE::SHADER);
	this->m_uuid = this->p_id_generate->generateUUID();
	this->m_initialized = true;
	
	Core::Core_Pool::addResource(m_uuid, this);

	initTemplate();
}

Core::Resource::RRenderPass::~RRenderPass()
{
	if (p_createInfo) delete p_createInfo;
	if (p_render_pass) {
		p_render_pass->removeFromVkObjects();
		delete p_render_pass;
	}
	vkDeviceWaitIdle(m_vkInfo.m_device);
	if (m_render_pass != VK_NULL_HANDLE) ThirdParty::vkManager.destroyRenderPass(m_render_pass);
}

void Core::Resource::RRenderPass::create()
{
	// ´¦Àísubpass
	m_subpass_descriptions.clear();
	for (const auto& subpass : m_subpass) {
		m_subpass_descriptions.push_back(subpass->getVkSubpassDescription());
	}

	if (!p_createInfo) {
		p_createInfo = new VkRenderPassCreateInfo;
		p_createInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		p_createInfo->flags = 0;
		p_createInfo->pNext = 0;
	}
	p_createInfo->attachmentCount = m_attachment_descs.size();
	p_createInfo->pAttachments = m_attachment_descs.data();
	p_createInfo->subpassCount = m_subpass_descriptions.size();
	p_createInfo->pSubpasses = m_subpass_descriptions.data();
	p_createInfo->dependencyCount = m_subpass_dependenies.size();
	p_createInfo->pDependencies = m_subpass_dependenies.data();

	m_render_pass = ThirdParty::vk_manager.createRenderPass(*p_createInfo);
	if (!p_render_pass) p_render_pass = new ThirdParty::RenderPass(m_render_pass, "RRenderPass RenderPass");
}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::addAttachment(VkImageLayout _finalLayout, VkImageLayout _initialLayout /*= VK_IMAGE_LAYOUT_UNDEFINED*/, VkFormat _format /*= VK_FORMAT_R8G8B8A8_UNORM*/, VkSampleCountFlagBits _samples /*= VK_SAMPLE_COUNT_1_BIT*/, VkAttachmentDescriptionFlags _flags /*= 0*/, VkAttachmentLoadOp _loadOp /*= VK_ATTACHMENT_LOAD_OP_CLEAR*/, VkAttachmentStoreOp _storeOp /*= VK_ATTACHMENT_STORE_OP_DONT_CARE*/, VkAttachmentLoadOp _stencilLoadOp /*= VK_ATTACHMENT_LOAD_OP_CLEAR*/, VkAttachmentStoreOp _stencilStoreOp /*= VK_ATTACHMENT_STORE_OP_DONT_CARE */)
{
	VkAttachmentDescription _a;
	{
		_a.flags = _flags;
		_a.format = _format;
		_a.samples = _samples;
		_a.loadOp = _loadOp;
		_a.storeOp = _storeOp;
		_a.stencilLoadOp = _stencilLoadOp;
		_a.stencilStoreOp = _stencilStoreOp;
		_a.initialLayout = _initialLayout;
		_a.finalLayout = _finalLayout;
	}
	m_attachment_descs.push_back(_a);
	return this;
}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::addNewSubpass()
{
	ThirdParty::Subpass* _s = new ThirdParty::Subpass("RRenderPass Subpass");
	this->m_subpass.push_back(_s);
	return this;
}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::setSubpassProperty_addColorAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index /*= -1*/)
{
	if (_sub_index < 0) _sub_index = m_subpass.size() - 1;
	m_subpass[_sub_index]->addColorAttachmentRef(_att_index, _layout);
	return this;
}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::setSubpassProperty_addDpethStencilAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index /*= -1*/)
{
	if (_sub_index < 0) _sub_index = m_subpass.size() - 1;
	m_subpass[_sub_index]->addDepthStencilAttachmentRef(_att_index, _layout);
	return this;
}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::setSubpassProperty_addResolveAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index /*= -1*/)
{
	if (_sub_index < 0) _sub_index = m_subpass.size() - 1;
	m_subpass[_sub_index]->addResolveAttachmentRef(_att_index, _layout);
	return this;
}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::addDependency(uint32_t _srcSubpass, uint32_t _dstSubpass, VkPipelineStageFlags _srcStageMask, VkPipelineStageFlags _dstStageMask, VkAccessFlags _srcAccessMask, VkAccessFlags _dstAccessMask, VkDependencyFlags _dependencyFlags)
{
	VkSubpassDependency _d = {
		_srcSubpass,
		_dstSubpass,
		_srcStageMask,
		_dstStageMask,
		_srcAccessMask,
		_dstAccessMask,
		_dependencyFlags
	};
	m_subpass_dependenies.push_back(_d);
	return this;
}

void Core::Resource::RRenderPass::initTemplate() {
	{
		// Output
		auto _output_0 = new ThirdParty::imgui::SlotTemplateOutput;
		_output_0->setName("RenderPass");

		p_template->addSlotTemplates({_output_0},false);
	}
}

Json::Value Core::Resource::RRenderPass::serializeToJSON()
{
	return Json::Value();
}

void Core::Resource::RRenderPass::deserializeToObj(Json::Value& root)
{

}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::getRRenderPass(const std::string& _name)
{
	return (RRenderPass*)Core::Core_Pool::g_all_renderpass[_name];
}

void Core::Resource::RRenderPass::addRRenderPass(const std::string& _name, RRenderPass* _renderpass)
{
	Core::Core_Pool::g_all_renderpass[_name] = _renderpass;
}

Core::Resource::RRenderPass* Core::Resource::RRenderPass::setSubpassProperty_addInputAttachment(uint32_t _att_index, VkImageLayout _layout, int _sub_index /*= -1*/)
{
	if (_sub_index < 0) _sub_index = m_subpass.size() - 1;
	m_subpass[_sub_index]->addInputAttachmentRef(_att_index, _layout);
	return this;
}
