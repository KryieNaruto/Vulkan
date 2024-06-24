#pragma once
#include <vector>
namespace ThirdParty {
	// 队列族
	struct QueueFamilyIndices {
		int graphicsFamily = -1;
		int presentFamily = -1;
		inline bool isComplete()
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};
	// 交换链细节
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	// 重建细节
	struct SwapchainRecreateDetails {
		VkSwapchainCreateInfoKHR m_swapchainInfo;
	};
}