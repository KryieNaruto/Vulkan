#pragma once
#include <ThirdParty/vulkan/vulkan.h>
#include "vk_struct.h"
#include <vector>
#include <string>
#include <map>
#include "core/info/include/application_window_info.h"
namespace ThirdParty {
	// 检查vkresult,VK_SUCCESS 返回ture,否则返回false
	bool ENGINE_API_THIRDPARTY checkVkResult(VkResult _result, const char* info);
	// vulkan校验层消息回调
	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	// 代理函数 PFN_vkCreateDebugUtilsMessengerEXT
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
	// 代理函数 PFN_vkDestroyDebugUtilsMessengerEXT
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);
	// 查找队列族
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physcialDevice);
	// 检查设备扩展是否可用
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	// 获取交换链细节
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	// 选择表面格式
	VkSurfaceFormatKHR chooseSurfaceFormatKHR(const std::vector<VkSurfaceFormatKHR>& formats);
	// 选择呈现
	VkPresentModeKHR choosePresentModeKHR(const std::vector<VkPresentModeKHR>& presentModes);
	// 选择交换范围
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	// 格式化字符串
	template<typename ... Args>
	std::string str_format(const std::string& str, Args ... args);
	// 二进制读取文件
	ENGINE_API_THIRDPARTY std::vector<char> readFile_binary(const std::string& filename);
	// 创建新的交换链图像视图,imageview会被clear
	void createSwapchainImageViews(VkFormat format, std::vector<VkImage>& images, std::vector<VkImageView>& imageViews);
	// 检查交换链是否过期
	ENGINE_API_THIRDPARTY void checkSwapchainUsable(VkResult result);
	// 计算vkformat的字长
	ENGINE_API_THIRDPARTY uint32_t getFormatSize(VkFormat _format);
	// 计算vkformat的rgb数量
	ENGINE_API_THIRDPARTY uint32_t getFormatType(VkFormat _format);
	// 获取vkformat的字符
	ENGINE_API_THIRDPARTY std::string getFormatStr(VkFormat _format);
	// 获取vkshaderstage的字符
	ENGINE_API_THIRDPARTY std::string getShaderStageStr(VkShaderStageFlagBits _stage);
	// 获取descriptor type的字符
	ENGINE_API_THIRDPARTY std::string getDescriptorType(VkDescriptorType _type);
	// 仅创建图像,不申请内存
	VkImageCreateInfo createImage(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImage& _image, VkSampleCountFlags _sample_count = VK_SAMPLE_COUNT_1_BIT, uint32_t _layer_count = 1, uint32_t _mip_map = 1);
	// 创建图像视图
	VkImageViewCreateInfo createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspect,VkImageView& _view,uint32_t _layer_count = 1,uint32_t _mip_map = 1);
	VkImageViewCreateInfo createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspect, VkImageView& _view, uint32_t _base_layer, uint32_t _layer_count = 1, uint32_t _base_mip = 0,uint32_t _mip_map = 1);
	// 申请图像设备内存
	VkDeviceMemory createImageDeviceMemory(size_t _size, uint32_t _memoryTypeBits, VkMemoryPropertyFlags _property);
	// 内存需求
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	// 返回屏障管线阶段
	ENGINE_API_THIRDPARTY VkPipelineStageFlags getPipelineStageFlags(VkImageLayout _layout);
	// 返回屏障掩码
	ENGINE_API_THIRDPARTY VkAccessFlags getAccessMask(VkImageLayout _layout);
	// 检测是否支持Bindless
	VkBool32 checkBindless();
	// 检测是否支持Device 扩展
	VkBool32 checkDeviceExtension(const std::string& _layer_name);
	ENGINE_API_THIRDPARTY VkFormat findSupportFormat(const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _flags);
	// 查找深度图布局
	ENGINE_API_THIRDPARTY VkFormat findDepthFormat();
	// 格式是否包含深度
	ENGINE_API_THIRDPARTY bool hasDepthComponent(VkFormat _format);
	// 格式是否包含模板
	ENGINE_API_THIRDPARTY bool hasStencilComponent(VkFormat _format);
	// 返回Image屏障所需对象
	std::pair<VkAccessFlags, VkPipelineStageFlags> getAccessMask_PipelineStage(VkImageLayout _layout);
	// 获取最大采样数
	VkSampleCountFlagBits getMaxUsableSampleCount();

	std::string getImageLayoutStr(VkImageLayout _layout);
	std::string getVkMemoryPropertyStr(VkMemoryPropertyFlags _f);
}