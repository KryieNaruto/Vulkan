#include "resource_preload.h"
#include <Core/include/core_global.h>
#include <ThirdPartyManager/core/info/include/global_info.h>
#include "shader_uniform_struct.h"
#include <model/RPlane.h>

VkAttachmentDescription Core::Resource::g_attachment_color = {
		0,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
};

void Core::Resource::preload()
{
	preload_sampler();
	preload_renderpass();
	preload_shape();
}


void Core::Resource::preload_renderpass()
{
	auto vkInfo = ThirdParty::Core::vkInfo;
	// RenderPass - subpass 0
	RRenderPass* p_render_normal = new RRenderPass();
	p_render_normal
		// 颜色附件，多重采样
		->addAttachment({
				0,
				VK_FORMAT_R8G8B8A8_UNORM,
				vkInfo.m_sample_count,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			})
		->addAttachment({
				0,
				ThirdParty::findDepthFormat(),
				vkInfo.m_sample_count,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
			})
		// 解析附件
		->addAttachment({
				0,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			})
		->addNewSubpass()
		->setSubpassProperty_addColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		->setSubpassProperty_addDpethStencilAttachment(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		->setSubpassProperty_addResolveAttachment(2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		->addDependency(
			VK_SUBPASS_EXTERNAL, 0,
			ThirdParty::getPipelineStageFlags(VK_IMAGE_LAYOUT_UNDEFINED),
			ThirdParty::getPipelineStageFlags(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),

			ThirdParty::getAccessMask(VK_IMAGE_LAYOUT_UNDEFINED),
			ThirdParty::getAccessMask(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
			0)
		->create();
	Core::Resource::Pool::addRRenderPassIntoPool(RENDER_PASS_0, p_render_normal);

	{
		RRenderPass* p_render_normal = new RRenderPass();
		p_render_normal
			// 颜色附件
			->addAttachment({
					0,
					VK_FORMAT_R8G8B8A8_UNORM,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				})
			->addNewSubpass()
			->setSubpassProperty_addColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			->addDependency(
				VK_SUBPASS_EXTERNAL, 0,
				ThirdParty::getPipelineStageFlags(VK_IMAGE_LAYOUT_UNDEFINED),
				ThirdParty::getPipelineStageFlags(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),

				ThirdParty::getAccessMask(VK_IMAGE_LAYOUT_UNDEFINED),
				ThirdParty::getAccessMask(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				0)
			->create();
		Core::Resource::Pool::addRRenderPassIntoPool(RENDER_PASS_NODEPTH, p_render_normal);
	}

	// RG without depth
	{
		RRenderPass* p_render_normal = new RRenderPass();
		p_render_normal
			// 颜色附件
			->addAttachment({
					0,
					VK_FORMAT_R8G8_UNORM,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				})
			->addNewSubpass()
			->setSubpassProperty_addColorAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			->addDependency(
				VK_SUBPASS_EXTERNAL, 0,
				ThirdParty::getPipelineStageFlags(VK_IMAGE_LAYOUT_UNDEFINED),
				ThirdParty::getPipelineStageFlags(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),

				ThirdParty::getAccessMask(VK_IMAGE_LAYOUT_UNDEFINED),
				ThirdParty::getAccessMask(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
				0)
			->create();
		Core::Resource::Pool::addRRenderPassIntoPool(RENDER_PASS_RG_NODEPTH, p_render_normal);
	}
}

void Core::Resource::preload_sampler()
{
	ThirdParty::Sampler* p_main_sampler = new ThirdParty::Sampler(SAMPLER);
	p_main_sampler->createSampler();
	ThirdParty::Sampler* p_main_sampler_repeat = new ThirdParty::Sampler(SAMPLER_REPEAT);
	p_main_sampler_repeat->createSampler(
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT
	);
	Core::Resource::Pool::addThirdParty_SamplerInfoPool(SAMPLER, p_main_sampler);
	Core::Resource::Pool::addThirdParty_SamplerInfoPool(SAMPLER_REPEAT, p_main_sampler_repeat);

	// glsl texture default
	ThirdParty::TextureEXT* p_default = new ThirdParty::TextureEXT(
		{ 1,1,1 },
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_LINEAR,
		VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_SAMPLE_COUNT_1_BIT,
		"default"
	);
	std::vector<uint32_t> data = { 0 };
	p_default->copyData(data.data());

	// skybox default
	std::string _res_img_skybox = "";
	_res_img_skybox.append(ThirdParty::Core::getModuleCurrentDirectory() + "\\resources\\img\\skybox");
	ThirdParty::TextureEXT* p_skybox = new ThirdParty::TextureEXT(
		{
			_res_img_skybox + "\\stormy-sky-skybox\\stormcloud_1_right.jpeg",
			_res_img_skybox + "\\stormy-sky-skybox\\stormcloud_3_left.jpeg",
			_res_img_skybox + "\\stormy-sky-skybox\\stormcloud_4_top.jpeg",
			_res_img_skybox + "\\stormy-sky-skybox\\stormcloud_5_down.jpeg",
			_res_img_skybox + "\\stormy-sky-skybox\\stormcloud_0_front.jpeg",
			_res_img_skybox + "\\stormy-sky-skybox\\stormcloud_2_back.jpeg",
		},"default-skybox"
	);

	// skybox hdri
	auto _hdri = _res_img_skybox + "\\cobblestone_street_night_4k.hdr";
	ThirdParty::TextureEXT* p_skybox_hdri = new ThirdParty::TextureEXT(_hdri, "default-skybox-hdri");
}

void Core::Resource::preload_shape()
{
}
