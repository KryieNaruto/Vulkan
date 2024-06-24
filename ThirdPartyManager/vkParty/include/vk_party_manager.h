#pragma once
#include "common/include/third_party_manager.h"
#include "vk_allocator.h"
#include "vk_struct.h"
#include <map>
#include <ThirdParty/glm/glm.hpp>
#include <unordered_map>
#include <variant>
namespace ThirdParty {
	//===================================
	// ������Vulkan��ͳһ����
	//===================================
	class ENGINE_API_THIRDPARTY VkManager :
		public Manager<VkManager>
	{
	public:
		uint32_t& getCurrentSwapchainImageIndex();
		uint32_t& getCurrentFrameIndex();
		// ����vkbuffer
		VkBuffer createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop, const std::vector<uint32_t> _queues = {});
		// ��Buffer memory
		// ����memory prop �����䵽���õ��ڴ��
		// ���������ڴ��
		VkDeviceMemory bindBufferMemory(VkBuffer _buffer, VkMemoryPropertyFlags _prop, size_t& _size, size_t& _offset);
		void unbindBufferMemory(VkDeviceMemory _mem, size_t _size, size_t _offset);
		// ���ڴ�ӳ��
		void bindBufferData(const VkDeviceMemory& _mem, void* _data,size_t _offset, size_t _size);
		//*********************************************************************************************************
		// ����-----------
		// �����豸�ڴ�
		VkDeviceMemory createBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags _prop);
		void destroyBufferMemory(VkDeviceMemory _mem);
		// ����vkbuffer [���ã����û��ʹ��ͬһ�ڴ��]
		void createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop, VkBuffer& _buffer, VkDeviceMemory& _mem,const std::vector<uint32_t> _queues = {});
		void destroyBuffer(VkBuffer& _buffer);
		//*********************************************************************************************************
		// ����shader fileΪVKShaderModule
		VkShaderModule createShaderModule(const std::string _path);
		// ����descriptorSet layout
		VkDescriptorSetLayout createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& _createInfo);
		// ����pipeline layout
		VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& _createInfo);
		// ���ض�����ΪVKShaderModule
		VkShaderModule createShaderModule(const std::vector<uint32_t> _binaries);
		// ����/����vkrenderpass
		VkRenderPass createRenderPass(const VkRenderPassCreateInfo& _c);
		void destroyRenderPass(VkRenderPass& _r);
		// ����shaderModule
		void destroyShaderModule(VkShaderModule& _module);
		// ����vkobject
		std::variant<VkImage,VkImageView,VkDeviceMemory>
			createVkObject(
				std::variant<
				VkImageCreateInfo*,
				VkImageViewCreateInfo*,
				VkMemoryAllocateInfo*
				> _ci
			);
		// ����vkobject
		void destroyVkObject(
			std::variant<VkImage,VkImageView,VkPipeline,VkPipelineLayout, VkDescriptorSetLayout> _o
		);
		// ����ͼ���豸�ڴ�,�����ڴ��С,�����ڴ����Ͳ����ڴ�飬��������ڣ��򴴽��ڴ�� API 1.1
		VkDeviceMemory bindImageMemory(VkImage _image, VkMemoryPropertyFlags _prop, size_t& size, size_t& offset);
		// ����ͼ���豸�ڴ�,�����ڴ��С	API 1.0
		void bindImageMemory(VkImage _image,size_t& size,size_t& offset);
		// �ͷ��ڴ�����	API 1.1
		void unbindImageMemory(VkDeviceMemory& _mem, size_t size, size_t offset);
		// �ͷ��ڴ����� API 1.0
		void unbindImageMemory(size_t size, size_t offset);
		// ����ϸ����ͼ
		void generateMipmaps(VkImage _image, const VkExtent3D& _size, uint32_t mipLevels,uint32_t _layers = 0);
		// ��������
		VkCommandBuffer BeginSingleCommandBuffer();
		void endSingleCommandBuffer();
		VkCommandBuffer BeginNewSigleCommandBuffer();
		void endSingleCommandBuffer(VkCommandBuffer _cmd);
	public:
		VkManager() = default;
		virtual ~VkManager();
		// ͨ�� Manager �̳�
		VkManager* init() override;
		VkManager* getManager() override;
		void destroy() override;
		void windowResize() override;

	protected:

	private:
		/************************************************************************/
		// API 1.0 ͼ�����
		// bind���ͷŵĿ��е��ڴ�����
		// key ����offset,
		// param �����С,
		std::map<size_t, size_t>	m_image_empty_memory;
		size_t						m_image_offset_current = 0;
		// R8G8B8A8_UNORM ��ʽ�ڴ�
		size_t PRE_IMAGE_MEMORY_SIZE = 4;	// �Խ�����ͼ���СΪ������4��Ԥ����ͼ���ڴ�
		VkDeviceMemory				m_image_memory	 = VK_NULL_HANDLE;
		/************************************************************************/
		
		/************************************************************************/
		// API 1.1 ͼ�����
		/************************************************************************/
		size_t PRE_IMAGE_MEMORY_SIZE_EXT = 4096 * 4096 * 4 * 4;		// 8�� 4096 * 4096 * rgba
		// ͳһ�ڴ��,key : memory type
		std::unordered_map<uint32_t, VkDeviceMemory> m_image_memory_ext;
		// bind���ͷŵĿ��е��ڴ�����
		// key �����ڴ���
		// param key ����offset
		// param param �����С
		std::unordered_map<VkDeviceMemory, std::unordered_map<size_t, size_t>> m_image_empty_memory_ext;
		// param : offset
		std::unordered_map<VkDeviceMemory, size_t> m_image_offset_current_ext;

		/************************************************************************/
		// ͳһ�ڴ��,Ԥ�����СΪglm::mat4 * 4096 * 4096
		const size_t PRE_BUFFER_MEMORY_SIZE = 4096 * 4096 * sizeof(glm::mat4);
		// ͳһ�ڴ�飬key: type bit
		std::unordered_map<uint32_t, VkDeviceMemory> m_buffer_memory;
		// bind���ͷŵĿ��е��ڴ�����
		// key �����ڴ���
		// param key ����offset
		// param param �����С
		std::unordered_map<VkDeviceMemory, std::unordered_map<size_t, size_t>> m_buffer_empty_memory;
		// param : offset
		std::unordered_map<VkDeviceMemory, size_t> m_buffer_offset_current;
		/************************************************************************/
		VkFence						m_fence_temp		  = VK_NULL_HANDLE;
		VkCommandBuffer				m_command_buffer_temp = VK_NULL_HANDLE;
	private:
		VkInstance					m_instance		 = VK_NULL_HANDLE;
		VkSurfaceKHR				m_surface		 = VK_NULL_HANDLE;
		VkSwapchainKHR				m_swapchain		 = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT	callback		 = VK_NULL_HANDLE;
		VkDevice					m_device		 = VK_NULL_HANDLE;
		VkPhysicalDevice			m_physical_device= VK_NULL_HANDLE;
		VkQueue						m_graphics_queue = VK_NULL_HANDLE;
		VkQueue						m_present_queue	 = VK_NULL_HANDLE;
		VkRenderPass				m_renderpass	 = VK_NULL_HANDLE;
		VkCommandPool				m_graphics_command_pool = VK_NULL_HANDLE;
		VkCommandPool				m_transfer_command_pool = VK_NULL_HANDLE;
		VkDescriptorPool			m_descriptor_pool	= VK_NULL_HANDLE;
		VkResult					result				= VK_ERROR_UNKNOWN;
		VkSampleCountFlagBits		m_sample_count		= VK_SAMPLE_COUNT_1_BIT;
		std::vector<VkImage>		m_swapchainImages;
		std::vector<VkImageView>	m_swapchain_imageViews;
		std::vector<VkFramebuffer>	m_swapchain_framebuffers;
		std::vector<VkSemaphore>	m_image_available_semaphores;
		std::vector<VkSemaphore>	m_render_finished_semaphores;
		std::vector<VkFence>		m_fences;
		// �ؽ���������Ϣ
		SwapchainRecreateDetails*	p_swapchain_recreate_info = nullptr;
		// vulkan �ڴ������
		Vk_Allocator* p_allocator = nullptr;

	};
	extern ENGINE_API_THIRDPARTY VkManager vkManager;
}

