#pragma once
#include <vector>
namespace ThirdParty {
	// ������
	struct QueueFamilyIndices {
		int graphicsFamily = -1;
		int presentFamily = -1;
		inline bool isComplete()
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};
	// ������ϸ��
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	// �ؽ�ϸ��
	struct SwapchainRecreateDetails {
		VkSwapchainCreateInfoKHR m_swapchainInfo;
	};
}