#pragma once
#include <ThirdParty/vulkan/vulkan.h>
#include "vk_struct.h"
#include <vector>
#include <string>
#include <map>
#include "core/info/include/application_window_info.h"
namespace ThirdParty {
	// ���vkresult,VK_SUCCESS ����ture,���򷵻�false
	bool ENGINE_API_THIRDPARTY checkVkResult(VkResult _result, const char* info);
	// vulkanУ�����Ϣ�ص�
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	// ������ PFN_vkCreateDebugUtilsMessengerEXT
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
	// ������ PFN_vkDestroyDebugUtilsMessengerEXT
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);
	// ���Ҷ�����
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physcialDevice);
	// ����豸��չ�Ƿ����
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	// ��ȡ������ϸ��
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	// ѡ������ʽ
	VkSurfaceFormatKHR chooseSurfaceFormatKHR(const std::vector<VkSurfaceFormatKHR>& formats);
	// ѡ�����
	VkPresentModeKHR choosePresentModeKHR(const std::vector<VkPresentModeKHR>& presentModes);
	// ѡ�񽻻���Χ
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	// ��ʽ���ַ���
	template<typename ... Args>
	std::string str_format(const std::string& str, Args ... args);
	// �����ƶ�ȡ�ļ�
	ENGINE_API_THIRDPARTY std::vector<char> readFile_binary(const std::string& filename);
	// �����µĽ�����ͼ����ͼ,imageview�ᱻclear
	void createSwapchainImageViews(VkFormat format, std::vector<VkImage>& images, std::vector<VkImageView>& imageViews);
	// ��齻�����Ƿ����
	ENGINE_API_THIRDPARTY void checkSwapchainUsable(VkResult result);
	// ����vkformat���ֳ�
	ENGINE_API_THIRDPARTY uint32_t getFormatSize(VkFormat _format);
	// ����vkformat��rgb����
	ENGINE_API_THIRDPARTY uint32_t getFormatType(VkFormat _format);
	// ��ȡvkformat���ַ�
	ENGINE_API_THIRDPARTY std::string getFormatStr(VkFormat _format);
	// ��ȡvkshaderstage���ַ�
	ENGINE_API_THIRDPARTY std::string getShaderStageStr(VkShaderStageFlagBits _stage);
	// ��ȡdescriptor type���ַ�
	ENGINE_API_THIRDPARTY std::string getDescriptorType(VkDescriptorType _type);
	// ������ͼ��,�������ڴ�
	VkImageCreateInfo createImage(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImage& _image, VkSampleCountFlags _sample_count = VK_SAMPLE_COUNT_1_BIT, uint32_t _layer_count = 1, uint32_t _mip_map = 1);
	// ����ͼ����ͼ
	VkImageViewCreateInfo createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspect,VkImageView& _view,uint32_t _layer_count = 1,uint32_t _mip_map = 1);
	VkImageViewCreateInfo createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspect, VkImageView& _view, uint32_t _base_layer, uint32_t _layer_count = 1, uint32_t _base_mip = 0,uint32_t _mip_map = 1);
	// ����ͼ���豸�ڴ�
	VkDeviceMemory createImageDeviceMemory(size_t _size, uint32_t _memoryTypeBits, VkMemoryPropertyFlags _property);
	// �ڴ�����
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	// �������Ϲ��߽׶�
	ENGINE_API_THIRDPARTY VkPipelineStageFlags getPipelineStageFlags(VkImageLayout _layout);
	// ������������
	ENGINE_API_THIRDPARTY VkAccessFlags getAccessMask(VkImageLayout _layout);
	// ����Ƿ�֧��Bindless
	VkBool32 checkBindless();
	// ����Ƿ�֧��Device ��չ
	VkBool32 checkDeviceExtension(const std::string& _layer_name);
	ENGINE_API_THIRDPARTY VkFormat findSupportFormat(const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _flags);
	// �������ͼ����
	ENGINE_API_THIRDPARTY VkFormat findDepthFormat();
	// ��ʽ�Ƿ�������
	ENGINE_API_THIRDPARTY bool hasDepthComponent(VkFormat _format);
	// ��ʽ�Ƿ����ģ��
	ENGINE_API_THIRDPARTY bool hasStencilComponent(VkFormat _format);
	// ����Image�����������
	std::pair<VkAccessFlags, VkPipelineStageFlags> getAccessMask_PipelineStage(VkImageLayout _layout);
	// ��ȡ��������
	VkSampleCountFlagBits getMaxUsableSampleCount();

	std::string getImageLayoutStr(VkImageLayout _layout);
	std::string getVkMemoryPropertyStr(VkMemoryPropertyFlags _f);
}