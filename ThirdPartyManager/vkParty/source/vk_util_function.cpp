#include "vk_util_function.h"
#include "core/info/include/global_info.h"
#include "core/common/include/debug_to_console.h"
#include <set>
#include <fstream>
#define RETURN_STR(value) case value: return #value;

bool ThirdParty::checkVkResult(VkResult _result, const char* info)
{
	if (_result != VK_SUCCESS) {
		Core::error(info);
		return false;
	}
	else return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL ThirdParty::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, const VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= Core::g_ApplicationVulkanInfo.m_message_severity) {
		if (messageType >= Core::g_ApplicationVulkanInfo.m_message_type) {
			if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
				Core::error(pCallbackData->pMessage);
			else if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				Core::warn(pCallbackData->pMessage);
			else Core::info(pCallbackData->pMessage);
		}
	}
	return VK_FALSE;
}

VkResult ThirdParty::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Core::g_ApplicationVulkanInfo.m_instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return  func(Core::g_ApplicationVulkanInfo.m_instance, pCreateInfo, pAllocator, pMessenger);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void ThirdParty::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Core::g_ApplicationVulkanInfo.m_instance, "vkDestroyDebugUtilsMessengerEXT");
	func(Core::g_ApplicationVulkanInfo.m_instance, messenger, pAllocator);
}

ThirdParty::QueueFamilyIndices ThirdParty::findQueueFamilies(VkPhysicalDevice physcialDevice)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physcialDevice, &queueFamilyCount, 0);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physcialDevice, &queueFamilyCount, queueFamilies.data());
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// 图形功能
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		// 呈现
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physcialDevice, i, Core::g_ApplicationVulkanInfo.m_surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}
		if (indices.isComplete()) break;
		i++;
	}


	return indices;
}

bool ThirdParty::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, 0);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, availableExtensions.data());
	std::set<const char*> requireExtensions(Core::g_ApplicationVulkanInfo.m_device_extensions.begin(), Core::g_ApplicationVulkanInfo.m_device_extensions.end());

	for (const auto& extension : availableExtensions) {
		requireExtensions.erase(extension.extensionName);
	}

	return requireExtensions.empty();
}

ThirdParty::SwapChainSupportDetails ThirdParty::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Core::g_ApplicationVulkanInfo.m_surface, &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, Core::g_ApplicationVulkanInfo.m_surface, &formatCount, 0);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, Core::g_ApplicationVulkanInfo.m_surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, Core::g_ApplicationVulkanInfo.m_surface, &presentModeCount, 0);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, Core::g_ApplicationVulkanInfo.m_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR ThirdParty::chooseSurfaceFormatKHR(const std::vector<VkSurfaceFormatKHR>& formats)
{
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		return { Core::g_ApplicationVulkanInfo.m_surface_format,Core::g_ApplicationVulkanInfo.m_surface_color_space };
	for (const auto& format : formats) {
		if (format.format == Core::g_ApplicationVulkanInfo.m_surface_format && format.colorSpace == Core::g_ApplicationVulkanInfo.m_surface_color_space)
			return format;
	}
	return formats[0];
}

VkPresentModeKHR ThirdParty::choosePresentModeKHR(const std::vector<VkPresentModeKHR>& presentModes)
{
	VkPresentModeKHR bestMode = Core::g_ApplicationVulkanInfo.m_surface_present_mode;
	for (const auto& presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			bestMode = presentMode;
			break;
		}
		else if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
			bestMode = presentMode;
			break;
		}
		else if (presentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
			bestMode = presentMode;
			break;
		}
		else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = presentMode;
		}
	}
	return bestMode;
}

VkExtent2D ThirdParty::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		Core::g_ApplicationVulkanInfo.m_swapchain_extent = capabilities.currentExtent;
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D extent = { Core::g_ApplicationWindowInfo.m_window_width, Core::g_ApplicationWindowInfo.m_window_height };
		extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent.width));
		extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent.height));
		Core::g_ApplicationVulkanInfo.m_swapchain_extent = extent;
		return extent;
	}
}

template<typename ... Args>
std::string ThirdParty::str_format(const std::string& str, Args ... args)
{
	auto buf_size = std::snprintf(nullptr, 0, str.c_str(), args...) + 1;
	std::unique_ptr<char[]> buf(new(std::nothrow) char[buf_size]);
	if (!buf)
		return std::string("");
	std::snprintf(buf.get(), buf_size, str.c_str(), args...);
	return std::string(buf.get(), buf.get() + buf_size - 1);
}


std::vector<char> ThirdParty::readFile_binary(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		Core::error(str_format("Failed to read file:%s", filename.c_str()).c_str());
		throw std::runtime_error(nullptr);
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void ThirdParty::createSwapchainImageViews(VkFormat format, std::vector<VkImage>& images, std::vector<VkImageView>& imageViews)
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	if (!imageViews.empty()) imageViews.clear();
	// 创建imageView
	for (const auto& swapchainImage : images) {
		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = swapchainImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
		viewInfo.pNext = nullptr;
		VkImageView view;
		VkResult result = vkCreateImageView(vkInfo->m_device, &viewInfo, vkInfo->p_allocate, &view);
		if (checkVkResult(result, "Failed to create swapchain image view")) {
			imageViews.push_back(view);
			Core::info("VkSwapchainImageView has been created");
		}
	}
}

void ThirdParty::checkSwapchainUsable(VkResult result)
{
	// 检查交换链
	{
		switch (result)
		{
		case VK_SUCCESS:
			break;
			// 交换链过期
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
			// 更新flag
			ThirdParty::Core::windowInfo.m_window_isResized = true;
			break;
		default:
			ThirdParty::Core::error("Swapchain has some errors");
			break;
		}
	}
}

#define SWITCH_GET_FORMAT_SIZE_1(_v,_t) \
case VK_FORMAT_R##_v##_##_t: return 1 * _v / 8;
#define SWITCH_GET_FORMAT_SIZE_2(_v,_t) \
case VK_FORMAT_R##_v##G##_v##_##_t: return 2 * _v / 8;
#define SWITCH_GET_FORMAT_SIZE_3(_v,_t) \
case VK_FORMAT_R##_v##G##_v##B##_v##_##_t: return 3 * _v / 8;
#define SWITCH_GET_FORMAT_SIZE_4(_v,_t) \
case VK_FORMAT_R##_v##G##_v##B##_v##A##_v##_##_t: return 4 * _v / 8;

uint32_t ThirdParty::getFormatSize(VkFormat _format)
{
	switch (_format)
	{
		SWITCH_GET_FORMAT_SIZE_1(8, UNORM);
		SWITCH_GET_FORMAT_SIZE_1(16, UNORM);
		SWITCH_GET_FORMAT_SIZE_1(16, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_1(32, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_1(64, SFLOAT);

		SWITCH_GET_FORMAT_SIZE_2(8, UNORM);
		SWITCH_GET_FORMAT_SIZE_2(16, UNORM);
		SWITCH_GET_FORMAT_SIZE_2(16, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_2(32, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_2(64, SFLOAT);

		SWITCH_GET_FORMAT_SIZE_3(8, UNORM);
		SWITCH_GET_FORMAT_SIZE_3(16, UNORM);
		SWITCH_GET_FORMAT_SIZE_3(16, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_3(32, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_3(64, SFLOAT);

		SWITCH_GET_FORMAT_SIZE_4(8, UNORM);
		SWITCH_GET_FORMAT_SIZE_4(16, UNORM);
		SWITCH_GET_FORMAT_SIZE_4(16, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_4(32, SFLOAT);
		SWITCH_GET_FORMAT_SIZE_4(64, SFLOAT);
	default:
		break;
	}
}

#define SWITCH_GET_FORMAT_TYPE_1(_v,_t) \
case VK_FORMAT_R##_v##_##_t: return 1;
#define SWITCH_GET_FORMAT_TYPE_2(_v,_t) \
case VK_FORMAT_R##_v##G##_v##_##_t: return 2;
#define SWITCH_GET_FORMAT_TYPE_3(_v,_t) \
case VK_FORMAT_R##_v##G##_v##B##_v##_##_t: return 3;
#define SWITCH_GET_FORMAT_TYPE_4(_v,_t) \
case VK_FORMAT_R##_v##G##_v##B##_v##A##_v##_##_t: return 4;

uint32_t ThirdParty::getFormatType(VkFormat _format)
{
	switch (_format)
	{
		SWITCH_GET_FORMAT_TYPE_1(8, UNORM);
		SWITCH_GET_FORMAT_TYPE_1(16, UNORM);
		SWITCH_GET_FORMAT_TYPE_1(16, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_1(32, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_1(64, SFLOAT);

		SWITCH_GET_FORMAT_TYPE_2(8, UNORM);
		SWITCH_GET_FORMAT_TYPE_2(16, UNORM);
		SWITCH_GET_FORMAT_TYPE_2(16, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_2(32, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_2(64, SFLOAT);

		SWITCH_GET_FORMAT_TYPE_3(8, UNORM);
		SWITCH_GET_FORMAT_TYPE_3(16, UNORM);
		SWITCH_GET_FORMAT_TYPE_3(16, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_3(32, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_3(64, SFLOAT);

		SWITCH_GET_FORMAT_TYPE_4(8, UNORM);
		SWITCH_GET_FORMAT_TYPE_4(16, UNORM);
		SWITCH_GET_FORMAT_TYPE_4(16, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_4(32, SFLOAT);
		SWITCH_GET_FORMAT_TYPE_4(64, SFLOAT);
	default:
		break;
	}
}

std::string ThirdParty::getFormatStr(VkFormat _format)
{
	switch (_format)
	{
	RETURN_STR( VK_FORMAT_UNDEFINED)
		
	RETURN_STR( VK_FORMAT_R4G4_UNORM_PACK8)
		
	RETURN_STR( VK_FORMAT_R4G4B4A4_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_B4G4R4A4_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_R5G6B5_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_B5G6R5_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_R5G5B5A1_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_B5G5R5A1_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_A1R5G5B5_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_R8_UNORM)
		
	RETURN_STR( VK_FORMAT_R8_SNORM)
		
	RETURN_STR( VK_FORMAT_R8_USCALED)
		
	RETURN_STR( VK_FORMAT_R8_SSCALED)
		
	RETURN_STR( VK_FORMAT_R8_UINT)
		
	RETURN_STR( VK_FORMAT_R8_SINT)
		
	RETURN_STR( VK_FORMAT_R8_SRGB)
		
	RETURN_STR( VK_FORMAT_R8G8_UNORM)
		
	RETURN_STR( VK_FORMAT_R8G8_SNORM)
		
	RETURN_STR( VK_FORMAT_R8G8_USCALED)
		
	RETURN_STR( VK_FORMAT_R8G8_SSCALED)
		
	RETURN_STR( VK_FORMAT_R8G8_UINT)
		
	RETURN_STR( VK_FORMAT_R8G8_SINT)
		
	RETURN_STR( VK_FORMAT_R8G8_SRGB)
		
	RETURN_STR( VK_FORMAT_R8G8B8_UNORM)
		
	RETURN_STR( VK_FORMAT_R8G8B8_SNORM)
		
	RETURN_STR( VK_FORMAT_R8G8B8_USCALED)
		
	RETURN_STR( VK_FORMAT_R8G8B8_SSCALED)
		
	RETURN_STR( VK_FORMAT_R8G8B8_UINT)
		
	RETURN_STR( VK_FORMAT_R8G8B8_SINT)
		
	RETURN_STR( VK_FORMAT_R8G8B8_SRGB)
		
	RETURN_STR( VK_FORMAT_B8G8R8_UNORM)
		
	RETURN_STR( VK_FORMAT_B8G8R8_SNORM)
		
	RETURN_STR( VK_FORMAT_B8G8R8_USCALED)
		
	RETURN_STR( VK_FORMAT_B8G8R8_SSCALED)
		
	RETURN_STR( VK_FORMAT_B8G8R8_UINT)
		
	RETURN_STR( VK_FORMAT_B8G8R8_SINT)
		
	RETURN_STR( VK_FORMAT_B8G8R8_SRGB)
		
	RETURN_STR( VK_FORMAT_R8G8B8A8_UNORM)
		
	RETURN_STR( VK_FORMAT_R8G8B8A8_SNORM)
		
	RETURN_STR( VK_FORMAT_R8G8B8A8_USCALED)
		
	RETURN_STR( VK_FORMAT_R8G8B8A8_SSCALED)
		
	RETURN_STR( VK_FORMAT_R8G8B8A8_UINT)
		
	RETURN_STR( VK_FORMAT_R8G8B8A8_SINT)
		
	RETURN_STR( VK_FORMAT_R8G8B8A8_SRGB)
		
	RETURN_STR( VK_FORMAT_B8G8R8A8_UNORM)
		
	RETURN_STR( VK_FORMAT_B8G8R8A8_SNORM)
		
	RETURN_STR( VK_FORMAT_B8G8R8A8_USCALED)
		
	RETURN_STR( VK_FORMAT_B8G8R8A8_SSCALED)
		
	RETURN_STR( VK_FORMAT_B8G8R8A8_UINT)
		
	RETURN_STR( VK_FORMAT_B8G8R8A8_SINT)
		
	RETURN_STR( VK_FORMAT_B8G8R8A8_SRGB)
		
	RETURN_STR( VK_FORMAT_A8B8G8R8_UNORM_PACK32)
		
	RETURN_STR( VK_FORMAT_A8B8G8R8_SNORM_PACK32)
		
	RETURN_STR( VK_FORMAT_A8B8G8R8_USCALED_PACK32)
		
	RETURN_STR( VK_FORMAT_A8B8G8R8_SSCALED_PACK32)
		
	RETURN_STR( VK_FORMAT_A8B8G8R8_UINT_PACK32)
		
	RETURN_STR( VK_FORMAT_A8B8G8R8_SINT_PACK32)
		
	RETURN_STR( VK_FORMAT_A8B8G8R8_SRGB_PACK32)
		
	RETURN_STR( VK_FORMAT_A2R10G10B10_UNORM_PACK32)
		
	RETURN_STR( VK_FORMAT_A2R10G10B10_SNORM_PACK32)
		
	RETURN_STR( VK_FORMAT_A2R10G10B10_USCALED_PACK32)
		
	RETURN_STR( VK_FORMAT_A2R10G10B10_SSCALED_PACK32)
		
	RETURN_STR( VK_FORMAT_A2R10G10B10_UINT_PACK32)
		
	RETURN_STR( VK_FORMAT_A2R10G10B10_SINT_PACK32)
		
	RETURN_STR( VK_FORMAT_A2B10G10R10_UNORM_PACK32)
		
	RETURN_STR( VK_FORMAT_A2B10G10R10_SNORM_PACK32)
		
	RETURN_STR( VK_FORMAT_A2B10G10R10_USCALED_PACK32)
		
	RETURN_STR( VK_FORMAT_A2B10G10R10_SSCALED_PACK32)
		
	RETURN_STR( VK_FORMAT_A2B10G10R10_UINT_PACK32)
		
	RETURN_STR( VK_FORMAT_A2B10G10R10_SINT_PACK32)
		
	RETURN_STR( VK_FORMAT_R16_UNORM)
		
	RETURN_STR( VK_FORMAT_R16_SNORM)
		
	RETURN_STR( VK_FORMAT_R16_USCALED)
		
	RETURN_STR( VK_FORMAT_R16_SSCALED)
		
	RETURN_STR( VK_FORMAT_R16_UINT)
		
	RETURN_STR( VK_FORMAT_R16_SINT)
		
	RETURN_STR( VK_FORMAT_R16_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R16G16_UNORM)
		
	RETURN_STR( VK_FORMAT_R16G16_SNORM)
		
	RETURN_STR( VK_FORMAT_R16G16_USCALED)
		
	RETURN_STR( VK_FORMAT_R16G16_SSCALED)
		
	RETURN_STR( VK_FORMAT_R16G16_UINT)
		
	RETURN_STR( VK_FORMAT_R16G16_SINT)
		
	RETURN_STR( VK_FORMAT_R16G16_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R16G16B16_UNORM)
		
	RETURN_STR( VK_FORMAT_R16G16B16_SNORM)
		
	RETURN_STR( VK_FORMAT_R16G16B16_USCALED)
		
	RETURN_STR( VK_FORMAT_R16G16B16_SSCALED)
		
	RETURN_STR( VK_FORMAT_R16G16B16_UINT)
		
	RETURN_STR( VK_FORMAT_R16G16B16_SINT)
		
	RETURN_STR( VK_FORMAT_R16G16B16_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R16G16B16A16_UNORM)
		
	RETURN_STR( VK_FORMAT_R16G16B16A16_SNORM)
		
	RETURN_STR( VK_FORMAT_R16G16B16A16_USCALED)
		
	RETURN_STR( VK_FORMAT_R16G16B16A16_SSCALED)
		
	RETURN_STR( VK_FORMAT_R16G16B16A16_UINT)
		
	RETURN_STR( VK_FORMAT_R16G16B16A16_SINT)
		
	RETURN_STR( VK_FORMAT_R16G16B16A16_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R32_UINT)
		
	RETURN_STR( VK_FORMAT_R32_SINT)
		
	RETURN_STR( VK_FORMAT_R32_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R32G32_UINT)
		
	RETURN_STR( VK_FORMAT_R32G32_SINT)
		
	RETURN_STR( VK_FORMAT_R32G32_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R32G32B32_UINT)
		
	RETURN_STR( VK_FORMAT_R32G32B32_SINT)
		
	RETURN_STR( VK_FORMAT_R32G32B32_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R32G32B32A32_UINT)
		
	RETURN_STR( VK_FORMAT_R32G32B32A32_SINT)
		
	RETURN_STR( VK_FORMAT_R32G32B32A32_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R64_UINT)
		
	RETURN_STR( VK_FORMAT_R64_SINT)
		
	RETURN_STR( VK_FORMAT_R64_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R64G64_UINT)
		
	RETURN_STR( VK_FORMAT_R64G64_SINT)
		
	RETURN_STR( VK_FORMAT_R64G64_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R64G64B64_UINT)
		
	RETURN_STR( VK_FORMAT_R64G64B64_SINT)
		
	RETURN_STR( VK_FORMAT_R64G64B64_SFLOAT)
		
	RETURN_STR( VK_FORMAT_R64G64B64A64_UINT)
		
	RETURN_STR( VK_FORMAT_R64G64B64A64_SINT)
		
	RETURN_STR( VK_FORMAT_R64G64B64A64_SFLOAT)
		
	RETURN_STR( VK_FORMAT_B10G11R11_UFLOAT_PACK32)
		
	RETURN_STR( VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
		
	RETURN_STR( VK_FORMAT_D16_UNORM)
		
	RETURN_STR( VK_FORMAT_X8_D24_UNORM_PACK32)
		
	RETURN_STR( VK_FORMAT_D32_SFLOAT)
		
	RETURN_STR( VK_FORMAT_S8_UINT)
		
	RETURN_STR( VK_FORMAT_D16_UNORM_S8_UINT)
		
	RETURN_STR( VK_FORMAT_D24_UNORM_S8_UINT)
		
	RETURN_STR( VK_FORMAT_D32_SFLOAT_S8_UINT)
		
	RETURN_STR( VK_FORMAT_BC1_RGB_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC1_RGB_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC1_RGBA_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC1_RGBA_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC2_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC2_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC3_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC3_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC4_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC4_SNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC5_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC5_SNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC6H_UFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC6H_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC7_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_BC7_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_EAC_R11_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_EAC_R11_SNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_EAC_R11G11_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_EAC_R11G11_SNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_4x4_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_4x4_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_5x4_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_5x4_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_5x5_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_5x5_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_6x5_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_6x5_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_6x6_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_6x6_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x5_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x5_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x6_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x6_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x8_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x8_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x5_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x5_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x6_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x6_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x8_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x8_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x10_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x10_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_12x10_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_12x10_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_12x12_UNORM_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_12x12_SRGB_BLOCK)
		
	RETURN_STR( VK_FORMAT_G8B8G8R8_422_UNORM)
		
	RETURN_STR( VK_FORMAT_B8G8R8G8_422_UNORM)
		
	RETURN_STR( VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)
		
	RETURN_STR( VK_FORMAT_G8_B8R8_2PLANE_420_UNORM)
		
	RETURN_STR( VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM)
		
	RETURN_STR( VK_FORMAT_G8_B8R8_2PLANE_422_UNORM)
		
	RETURN_STR( VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM)
		
	RETURN_STR( VK_FORMAT_R10X6_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_R10X6G10X6_UNORM_2PACK16)
		
	RETURN_STR( VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16)
		
	RETURN_STR( VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16)
		
	RETURN_STR( VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16)
		
	RETURN_STR( VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_R12X4_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_R12X4G12X4_UNORM_2PACK16)
		
	RETURN_STR( VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16)
		
	RETURN_STR( VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16)
		
	RETURN_STR( VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16)
		
	RETURN_STR( VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G16B16G16R16_422_UNORM)
		
	RETURN_STR( VK_FORMAT_B16G16R16G16_422_UNORM)
		
	RETURN_STR( VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM)
		
	RETURN_STR( VK_FORMAT_G16_B16R16_2PLANE_420_UNORM)
		
	RETURN_STR( VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM)
		
	RETURN_STR( VK_FORMAT_G16_B16R16_2PLANE_422_UNORM)
		
	RETURN_STR( VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM)
		
	RETURN_STR( VK_FORMAT_G8_B8R8_2PLANE_444_UNORM)
		
	RETURN_STR( VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16)
		
	RETURN_STR( VK_FORMAT_G16_B16R16_2PLANE_444_UNORM)
		
	RETURN_STR( VK_FORMAT_A4R4G4B4_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_A4B4G4R4_UNORM_PACK16)
		
	RETURN_STR( VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK)
		
	RETURN_STR( VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG)
		
	RETURN_STR( VK_FORMAT_R16G16_S10_5_NV)
		
	RETURN_STR( VK_FORMAT_MAX_ENUM)
		
	default:
		return "Unknow";
	}
}

std::string ThirdParty::getShaderStageStr(VkShaderStageFlagBits _stage)
{
	switch (_stage)
	{
	RETURN_STR(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)

	RETURN_STR( VK_SHADER_STAGE_VERTEX_BIT)
		
	RETURN_STR( VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
		
	RETURN_STR( VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
		
	RETURN_STR( VK_SHADER_STAGE_GEOMETRY_BIT)
		
	RETURN_STR( VK_SHADER_STAGE_FRAGMENT_BIT)
		
	RETURN_STR( VK_SHADER_STAGE_COMPUTE_BIT)
		
	RETURN_STR( VK_SHADER_STAGE_ALL_GRAPHICS)
		
	RETURN_STR( VK_SHADER_STAGE_ALL)
		
	RETURN_STR( VK_SHADER_STAGE_RAYGEN_BIT_KHR)
		
	RETURN_STR( VK_SHADER_STAGE_ANY_HIT_BIT_KHR)
		
	RETURN_STR( VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
		
	RETURN_STR( VK_SHADER_STAGE_MISS_BIT_KHR)
		
	RETURN_STR( VK_SHADER_STAGE_INTERSECTION_BIT_KHR)
		
	RETURN_STR( VK_SHADER_STAGE_CALLABLE_BIT_KHR)
		
	RETURN_STR( VK_SHADER_STAGE_TASK_BIT_EXT)
		
	RETURN_STR( VK_SHADER_STAGE_MESH_BIT_EXT)
		
	RETURN_STR( VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI)
		
	RETURN_STR( VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI)
		
	default:
		return "Unknow";
		
	}
}

std::string ThirdParty::getDescriptorType(VkDescriptorType _type)
{
	switch (_type)
	{
	RETURN_STR( VK_DESCRIPTOR_TYPE_SAMPLER)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_MUTABLE_EXT)
		
	RETURN_STR( VK_DESCRIPTOR_TYPE_MAX_ENUM)
		
	default:
		return "Unknow";
	}
}

VkImageCreateInfo ThirdParty::createImage(const VkExtent3D& _extent, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _prop, VkImage& _image, VkSampleCountFlags _sample_count, uint32_t _layer_count, uint32_t _mip_map)
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	VkImageCreateInfo _image_create_info;
	// 创建vkimage
	_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	_image_create_info.imageType = VK_IMAGE_TYPE_2D;
	_image_create_info.extent = _extent;
	_image_create_info.mipLevels = _mip_map;
	_image_create_info.arrayLayers = _layer_count;
	_image_create_info.format = _format;
	_image_create_info.tiling = _tiling;
	_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	_image_create_info.usage = _usage;
	_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	_image_create_info.queueFamilyIndexCount = 0;
	_image_create_info.pQueueFamilyIndices = 0;
	_image_create_info.samples = (VkSampleCountFlagBits)_sample_count;
	_image_create_info.flags = _layer_count == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	_image_create_info.pNext = 0;
	vkCreateImage(vkInfo->m_device, &_image_create_info, vkInfo->p_allocate, &_image);
	return _image_create_info;
}

VkImageViewCreateInfo ThirdParty::createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspect,VkImageView& _view, uint32_t _layer_count, uint32_t _mip_map)
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	VkImageViewCreateInfo _image_view_create_info;
	// 创建vkImageView
	_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_image_view_create_info.image = _image;
	_image_view_create_info.viewType = _layer_count == 6 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	_image_view_create_info.format = _format;
	_image_view_create_info.subresourceRange = { _aspect,0,_mip_map,0,_layer_count };
	_image_view_create_info.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	_image_view_create_info.flags = 0;
	_image_view_create_info.pNext = 0;
	vkCreateImageView(vkInfo->m_device, &_image_view_create_info, vkInfo->p_allocate, &_view);
	return _image_view_create_info;
}

VkImageViewCreateInfo ThirdParty::createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspect, VkImageView& _view, uint32_t _base_layer, uint32_t _layer_count, uint32_t _base_mip, uint32_t _mip_map)
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	VkImageViewCreateInfo _image_view_create_info;
	// 创建vkImageView
	_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	_image_view_create_info.image = _image;
	_image_view_create_info.viewType = _layer_count == 6 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	_image_view_create_info.format = _format;
	_image_view_create_info.subresourceRange = { _aspect,_base_mip,_mip_map,_base_layer,_layer_count };
	_image_view_create_info.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	_image_view_create_info.flags = 0;
	_image_view_create_info.pNext = 0;
	vkCreateImageView(vkInfo->m_device, &_image_view_create_info, vkInfo->p_allocate, &_view);
	return _image_view_create_info;
}

VkDeviceMemory ThirdParty::createImageDeviceMemory(size_t _size, uint32_t _memoryTypeBits, VkMemoryPropertyFlags _property)
{
	VkDeviceMemory mem;
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	VkMemoryAllocateInfo a = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	a.allocationSize = _size;
	a.memoryTypeIndex = findMemoryType(_memoryTypeBits, _property);
	VkResult result = vkAllocateMemory(vkInfo->m_device, &a, vkInfo->p_allocate, &mem);
	if (checkVkResult(result, "Failed to create image memory"))
		return mem;
	return VK_NULL_HANDLE;
}

uint32_t ThirdParty::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vkInfo->m_physicalDevices[vkInfo->m_physicalIndex],&memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	ThirdParty::Core::error(("Failed to find suitable memory type:"+getVkMemoryPropertyStr(properties)).c_str());
	return 0xFF;
}

VkPipelineStageFlags ThirdParty::getPipelineStageFlags(VkImageLayout _layout)
{
	return getAccessMask_PipelineStage(_layout).second;
}

VkAccessFlags ThirdParty::getAccessMask(VkImageLayout _layout)
{
	return getAccessMask_PipelineStage(_layout).first;
}

VkBool32 ThirdParty::checkBindless()
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;

	VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
	indexingFeatures.pNext = nullptr;
	VkPhysicalDeviceFeatures2 deviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures2.pNext = &indexingFeatures;
	vkGetPhysicalDeviceFeatures2(vkInfo->m_physicalDevices[vkInfo->m_physicalIndex], &deviceFeatures2);

	if (indexingFeatures.runtimeDescriptorArray &&	// 启用动态数组，[]
		indexingFeatures.descriptorBindingVariableDescriptorCount &&  // 启用在DescriptorSet Layout的Binding中使用可变大小的AoD
		indexingFeatures.shaderSampledImageArrayNonUniformIndexing	// 启用SPIR-V中通过NonUniformEXT Decoration 用于非Uniform变量下表索引资源数组
		)
		return true;
	else return false;
}

VkBool32 ThirdParty::checkDeviceExtension(const std::string& _layer_name)
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;

	uint32_t _ext_count = 0;
	// 获取所有extension列表
	vkEnumerateDeviceExtensionProperties(vkInfo->m_physicalDevices[vkInfo->m_physicalIndex], 0, &_ext_count, 0);
	std::vector<VkExtensionProperties> _exts(_ext_count);
	vkEnumerateDeviceExtensionProperties(vkInfo->m_physicalDevices[vkInfo->m_physicalIndex], 0, &_ext_count, _exts.data());
	for (const auto& _ext : _exts) {
		if (_ext.extensionName == _layer_name)
			return true;
	}
	return false;
}

VkFormat ThirdParty::findSupportFormat(const std::vector<VkFormat>& _formats, VkImageTiling _tiling, VkFormatFeatureFlags _flags)
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	for (const auto& _format : _formats) {
		VkFormatProperties _prop;
		vkGetPhysicalDeviceFormatProperties(vkInfo->m_physicalDevices[vkInfo->m_physicalIndex], _format, &_prop);
		if (_tiling == VK_IMAGE_TILING_LINEAR && (_prop.linearTilingFeatures & _tiling) == _tiling) return _format;
		if (_tiling == VK_IMAGE_TILING_OPTIMAL && (_prop.optimalTilingFeatures & _tiling) == _tiling) return _format;
		Core::error("Failed to find supported format!");
	}
}

VkFormat ThirdParty::findDepthFormat()
{
	return findSupportFormat(
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
		},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
}

bool ThirdParty::hasDepthComponent(VkFormat _format)
{
	return
		_format == VK_FORMAT_D32_SFLOAT ||
		_format == VK_FORMAT_D24_UNORM_S8_UINT ||
		_format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		_format == VK_FORMAT_D24_UNORM_S8_UINT;
}

bool ThirdParty::hasStencilComponent(VkFormat _format)
{
	return _format == VK_FORMAT_D24_UNORM_S8_UINT || _format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

std::pair<VkAccessFlags, VkPipelineStageFlags> ThirdParty::getAccessMask_PipelineStage(VkImageLayout _layout)
{
	VkPipelineStageFlags stage;
	VkAccessFlags mask;
	switch (_layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		mask = 0;
		stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		mask = VK_ACCESS_SHADER_READ_BIT;
		stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		mask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		mask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		mask = VK_ACCESS_TRANSFER_READ_BIT;
		stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	default:
		mask = 0;
		stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		Core::warn("Current layout has not been supported: %s", getImageLayoutStr(_layout).c_str());
		break;
	}
	return std::make_pair(mask, stage);
}

VkSampleCountFlagBits ThirdParty::getMaxUsableSampleCount()
{
	const auto& vkInfo = &Core::g_ApplicationVulkanInfo;
	VkPhysicalDeviceProperties _phyProp;
	vkGetPhysicalDeviceProperties(vkInfo->m_physicalDevices[vkInfo->m_physicalIndex], &_phyProp);

	VkSampleCountFlags _count = std::min(_phyProp.limits.framebufferColorSampleCounts, _phyProp.limits.framebufferDepthSampleCounts);
	if (_count & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (_count & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (_count & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (_count & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (_count & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (_count & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	
	return VK_SAMPLE_COUNT_1_BIT;
}

std::string ThirdParty::getImageLayoutStr(VkImageLayout _layout)
{
	switch (_layout)
	{
		RETURN_STR(VK_IMAGE_LAYOUT_UNDEFINED);
		RETURN_STR(VK_IMAGE_LAYOUT_GENERAL);
		RETURN_STR(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_PREINITIALIZED);
		RETURN_STR(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
		RETURN_STR(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		RETURN_STR(VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR);
		RETURN_STR(VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR);
		RETURN_STR(VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR);
		RETURN_STR(VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR);
		RETURN_STR(VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT);
		RETURN_STR(VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR);
	default:
		return "UNKNOW_IMAGE_LAYOUT";
	}
}

std::string ThirdParty::getVkMemoryPropertyStr(VkMemoryPropertyFlags _f)
{
	switch (_f)
	{
		RETURN_STR(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		RETURN_STR(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		RETURN_STR(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		RETURN_STR(VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
		RETURN_STR(VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
		RETURN_STR(VK_MEMORY_PROPERTY_PROTECTED_BIT);
		RETURN_STR(VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
		RETURN_STR(VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD);
		RETURN_STR(VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV);
		RETURN_STR(VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM);
	default:
		break;
	}
}
