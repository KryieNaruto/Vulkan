#include "vkobject/vk_sampler.h"

ThirdParty::Sampler::~Sampler()
{
	if (p_create_info) delete p_create_info;
	if (m_sampler != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(m_vkInfo.m_device);
		vkDestroySampler(m_vkInfo.m_device, m_sampler, m_vkInfo.p_allocate);
		m_sampler = VK_NULL_HANDLE;
	}
}

ThirdParty::Sampler* ThirdParty::Sampler::createSampler(VkFilter _magFilter /*= VK_FILTER_LINEAR*/, VkFilter _minFilter /*= VK_FILTER_LINEAR*/, VkSamplerAddressMode _addressModeU /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/, VkSamplerAddressMode _addressModeV /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/, VkSamplerAddressMode _addressModeW /*= VK_SAMPLER_ADDRESS_MODE_REPEAT*/, VkBool32 _anisotropyEnable /*= VK_TRUE*/, float _maxAnisotropy /*= 16*/, VkBool32 _compareEnable /*= VK_FALSE*/, VkCompareOp _compareOp /*= VK_COMPARE_OP_ALWAYS*/, VkSamplerMipmapMode _mipmapMode /*= VK_SAMPLER_MIPMAP_MODE_LINEAR*/, float _mipLodBias /*= 0.0f*/, float _minLod /*= 0.0f*/, float _maxLod /*= 0.0f*/, VkBorderColor _borderColor /*= VK_BORDER_COLOR_INT_OPAQUE_BLACK*/, VkBool32 _unnormalizedCoordinates /*= VK_FALSE */)
{
	if (!p_create_info) {
		p_create_info = new VkSamplerCreateInfo;
		p_create_info->sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		p_create_info->flags = 0;
		p_create_info->pNext = 0;
	}
	p_create_info->magFilter = _magFilter;
	p_create_info->minFilter = _minFilter;
	p_create_info->addressModeU = _addressModeU;
	p_create_info->addressModeV = _addressModeV;
	p_create_info->addressModeW = _addressModeW;
	p_create_info->anisotropyEnable = _anisotropyEnable;
	p_create_info->maxAnisotropy = _maxAnisotropy;
	p_create_info->compareEnable = _compareEnable;
	p_create_info->compareOp = _compareOp;
	p_create_info->mipmapMode = _mipmapMode;
	p_create_info->mipLodBias = _mipLodBias;
	p_create_info->minLod = _minLod;
	p_create_info->maxLod = _maxLod;
	p_create_info->borderColor = _borderColor;
	p_create_info->unnormalizedCoordinates = _unnormalizedCoordinates;

	m_result = vkCreateSampler(m_vkInfo.m_device, p_create_info, m_vkInfo.p_allocate, &m_sampler);
	checkVkResult(m_result, ("Failed to create vkSampler:" + m_label).c_str());
	g_all_samplers[m_label] = this;
	return this;
}
