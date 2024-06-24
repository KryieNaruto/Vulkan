#pragma once
#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#pragma warning( disable: 4251 )

#ifdef _WIN32
    #ifdef THIRDPARTYMANAGER_EXPORTS
        #define ENGINE_API_THIRDPARTY __declspec(dllexport)
    #else
        #define ENGINE_API_THIRDPARTY __declspec(dllimport)
    #endif // THIRDPARTYMANAGER_EXPORTS
#else
#ifdef THIRDPARTYMANAGER_EXPORTS
#define ENGINE_API_THIRDPARTY __attribute__((__visibility__ ("default")))
#else
#define ENGINE_API_THIRDPARTY __attribute__((__visibility__ ("hidden")))
#endif // THIRDPARTYMANAGER_EXPORTS
#endif

namespace ThirdParty {
	namespace Core {
		enum  ENGINE_API_THIRDPARTY API_VERSION {
			API_1_0 = 0x00,
			API_1_1 = 0x01,
		};

		//========================
		// Ӧ�ó�����Ϣ
		//========================
		struct ENGINE_API_THIRDPARTY ApplicationInfo {
			API_VERSION m_api = API_1_1;
			bool m_debug;	// ����ģʽ
			bool m_multiView;	// imgui multi-view
			bool m_isSaved = false;
			bool m_saving = false;
			bool m_opening = false;
			ApplicationInfo(bool _debug = false, bool _multiView = false);
		};

		//========================
		// ���򴰿���Ϣ
		//========================
		struct ENGINE_API_THIRDPARTY ApplicationWindowInfo
		{
			ApplicationWindowInfo(const char* title, int flags);
			ApplicationWindowInfo updateWindowInfo(SDL_Window* p_window = nullptr);
			std::string m_title;
			uint32_t m_window_width;
			uint32_t m_window_height;
			float m_window_aspect;
			uint32_t m_window_sdl_flags;
			bool m_window_isClosed;
			bool m_window_isResized;
			bool m_window_isMinimized;
		};

		//========================
		// SDL ��Ϣ
		//========================
		struct ENGINE_API_THIRDPARTY ApplicationSDLInfo {
			SDL_Window* p_sdlwindow = nullptr;
			SDL_Event* p_sdlevent = nullptr;
		};

		//========================
		// VK��Ϣ
		//========================
		struct ENGINE_API_THIRDPARTY ApplicationVulkanInfo {
			// ֡����+1
			void nextFrame();
			// ���submitInfo,��nextFrame ���ƣ�û��Ϊframe_index + 1
			void newFrame();
			ApplicationVulkanInfo* removeCommandBuffer(const VkCommandBuffer& _v);
			ApplicationVulkanInfo* addCommandBuffer(const std::vector<VkCommandBuffer>& _v);
			ApplicationVulkanInfo* addWaitPipelineStage(const std::vector<VkPipelineStageFlags>& _v);
			ApplicationVulkanInfo* addWaitSemaphores(const std::vector<VkSemaphore>& _v);
			ApplicationVulkanInfo* addSignalSemaphores(const std::vector<VkSemaphore>& _v);

			std::vector<const char*>		m_instance_extensions;	// instance ��չ
			std::vector<const char*>		m_device_extensions;	// device ��չ
			std::vector<const char*>		m_validation_layers;	// У�����չ
			std::vector<VkPhysicalDevice>	m_physicalDevices;
			//===============
			// �����ύ
			std::vector<VkCommandBuffer> m_submit_command_buffers;
			std::vector<VkPipelineStageFlags> m_submit_wait_pipeline_stages;
			std::vector<VkSemaphore> m_submit_wait_semaphores;
			std::vector<VkSemaphore> m_submit_signal_semaphores;
			//===============
			// ��Swapchain
			std::vector<VkImage>			m_swapchain_images;
			std::vector<VkImageView>		m_swapchain_imageViews;
			std::vector<VkFramebuffer>		m_swapchain_framebuffers;
			std::vector<VkCommandBuffer>	m_main_command_buffers;
			std::vector<VkSemaphore>		m_image_available_semaphores;
			std::vector<VkSemaphore>		m_render_finished_semaphores;
			std::vector<VkFence>			m_fences;
			//===============
			uint32_t						m_graphics_queue_family_indices;
			uint32_t						m_present_queue_family_indices;
			uint32_t						m_transfer_queue_family_indices;
			uint8_t							m_physicalIndex = 0;
			uint32_t						m_frame_count = 0;
			uint32_t						m_current_frame = 0;
			uint32_t						m_current_image = 0;
			uint32_t						m_max_img_descriptor_count = 64;
			uint32_t						m_max_ubo_descriptor_count = 64;
			VkInstance						m_instance = VK_NULL_HANDLE;
			uint16_t						m_message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;	// ��Ϣ����
			uint16_t						m_message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;		// ��Ϣ����
			VkDevice						m_device = VK_NULL_HANDLE;
			VkQueue							m_graphics_queue = VK_NULL_HANDLE;
			VkQueue							m_present_queue = VK_NULL_HANDLE;
			VkQueue							m_transfer_queue = VK_NULL_HANDLE;
			VkSurfaceKHR					m_surface = VK_NULL_HANDLE;
			VkSwapchainKHR					m_swapchain = VK_NULL_HANDLE;
			VkDescriptorPool				m_descriptor_pool = VK_NULL_HANDLE;
			VkCommandPool					m_graphics_command_pool = VK_NULL_HANDLE;
			VkCommandPool					m_transfer_command_pool = VK_NULL_HANDLE;
			VkRenderPass					m_renderpass = VK_NULL_HANDLE;
			VkSampleCountFlagBits			m_sample_count = VK_SAMPLE_COUNT_1_BIT;
			VkFormat						m_surface_format = VK_FORMAT_R8G8B8A8_UNORM;
			VkColorSpaceKHR					m_surface_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			VkPresentModeKHR				m_surface_present_mode = VK_PRESENT_MODE_FIFO_KHR;
			VkExtent2D						m_swapchain_extent = { 800,600 };
			VkAllocationCallbacks*			p_allocate = nullptr;
			// ����֧��
			VkBool32						m_bindless = VK_FALSE;
			// ��̬��Ⱦ
			VkBool32						m_dynamic_renderpass = VK_FALSE;
		};
	}
}