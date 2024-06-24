#pragma once
#include "vk_object.h"
#include <unordered_map>
namespace ThirdParty {

    class ENGINE_API_THIRDPARTY Sampler :
        public vkObject
    {
    public:
		inline Sampler(const std::string& _label = "Sampler") :vkObject(_label) {}
		inline static std::unordered_map<std::string, Sampler*> g_all_samplers;
        virtual ~Sampler();
		Sampler* createSampler(
			VkFilter                _magFilter = VK_FILTER_LINEAR,
			VkFilter                _minFilter = VK_FILTER_LINEAR,
			VkSamplerAddressMode    _addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			VkSamplerAddressMode    _addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			VkSamplerAddressMode    _addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			VkBool32                _anisotropyEnable = VK_TRUE,
			float                   _maxAnisotropy = 16,
			VkBool32                _compareEnable = VK_FALSE,
			VkCompareOp             _compareOp = VK_COMPARE_OP_ALWAYS,
			VkSamplerMipmapMode     _mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			float                   _mipLodBias = 0.0f,
			float                   _minLod = 0.0f,
			float                   _maxLod = 16.0f,
			VkBorderColor           _borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			VkBool32                _unnormalizedCoordinates = VK_FALSE
		);
        inline VkSampler& getVkSampler() { return m_sampler; }
    private:
        VkSamplerCreateInfo* p_create_info = nullptr;
        VkSampler m_sampler = VK_NULL_HANDLE;
    };

}

