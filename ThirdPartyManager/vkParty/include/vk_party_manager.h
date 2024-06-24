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
	// 包含对Vulkan的统一操作
	//===================================
	class ENGINE_API_THIRDPARTY VkManager :
		public Manager<VkManager>
	{
	public:
		uint32_t& getCurrentSwapchainImageIndex();
		uint32_t& getCurrentFrameIndex();
		// 创建vkbuffer
		VkBuffer createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop, const std::vector<uint32_t> _queues = {});
		// 绑定Buffer memory
		// 根据memory prop 来分配到可用的内存块
		// 返回所属内存块
		VkDeviceMemory bindBufferMemory(VkBuffer _buffer, VkMemoryPropertyFlags _prop, size_t& _size, size_t& _offset);
		void unbindBufferMemory(VkDeviceMemory _mem, size_t _size, size_t _offset);
		// 绑定内存映射
		void bindBufferData(const VkDeviceMemory& _mem, void* _data,size_t _offset, size_t _size);
		//*********************************************************************************************************
		// 弃用-----------
		// 申请设备内存
		VkDeviceMemory createBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags _prop);
		void destroyBufferMemory(VkDeviceMemory _mem);
		// 创建vkbuffer [弃用，这个没有使用同一内存块]
		void createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop, VkBuffer& _buffer, VkDeviceMemory& _mem,const std::vector<uint32_t> _queues = {});
		void destroyBuffer(VkBuffer& _buffer);
		//*********************************************************************************************************
		// 加载shader file为VKShaderModule
		VkShaderModule createShaderModule(const std::string _path);
		// 创建descriptorSet layout
		VkDescriptorSetLayout createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& _createInfo);
		// 创建pipeline layout
		VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& _createInfo);
		// 加载二进制为VKShaderModule
		VkShaderModule createShaderModule(const std::vector<uint32_t> _binaries);
		// 创建/销毁vkrenderpass
		VkRenderPass createRenderPass(const VkRenderPassCreateInfo& _c);
		void destroyRenderPass(VkRenderPass& _r);
		// 销毁shaderModule
		void destroyShaderModule(VkShaderModule& _module);
		// 创建vkobject
		std::variant<VkImage,VkImageView,VkDeviceMemory>
			createVkObject(
				std::variant<
				VkImageCreateInfo*,
				VkImageViewCreateInfo*,
				VkMemoryAllocateInfo*
				> _ci
			);
		// 销毁vkobject
		void destroyVkObject(
			std::variant<VkImage,VkImageView,VkPipeline,VkPipelineLayout, VkDescriptorSetLayout> _o
		);
		// 申请图像设备内存,返回内存大小,根据内存类型查找内存块，如果不存在，则创建内存块 API 1.1
		VkDeviceMemory bindImageMemory(VkImage _image, VkMemoryPropertyFlags _prop, size_t& size, size_t& offset);
		// 申请图像设备内存,返回内存大小	API 1.0
		void bindImageMemory(VkImage _image,size_t& size,size_t& offset);
		// 释放内存区间	API 1.1
		void unbindImageMemory(VkDeviceMemory& _mem, size_t size, size_t offset);
		// 释放内存区间 API 1.0
		void unbindImageMemory(size_t size, size_t offset);
		// 生成细化贴图
		void generateMipmaps(VkImage _image, const VkExtent3D& _size, uint32_t mipLevels,uint32_t _layers = 0);
		// 单次命令
		VkCommandBuffer BeginSingleCommandBuffer();
		void endSingleCommandBuffer();
		VkCommandBuffer BeginNewSigleCommandBuffer();
		void endSingleCommandBuffer(VkCommandBuffer _cmd);
	public:
		VkManager() = default;
		virtual ~VkManager();
		// 通过 Manager 继承
		VkManager* init() override;
		VkManager* getManager() override;
		void destroy() override;
		void windowResize() override;

	protected:

	private:
		/************************************************************************/
		// API 1.0 图像分配
		// bind后被释放的空闲的内存区间
		// key 代表offset,
		// param 代表大小,
		std::map<size_t, size_t>	m_image_empty_memory;
		size_t						m_image_offset_current = 0;
		// R8G8B8A8_UNORM 格式内存
		size_t PRE_IMAGE_MEMORY_SIZE = 4;	// 以交换链图像大小为计数，4张预定义图像内存
		VkDeviceMemory				m_image_memory	 = VK_NULL_HANDLE;
		/************************************************************************/
		
		/************************************************************************/
		// API 1.1 图像分配
		/************************************************************************/
		size_t PRE_IMAGE_MEMORY_SIZE_EXT = 4096 * 4096 * 4 * 4;		// 8张 4096 * 4096 * rgba
		// 统一内存块,key : memory type
		std::unordered_map<uint32_t, VkDeviceMemory> m_image_memory_ext;
		// bind后被释放的空闲的内存区间
		// key 代表内存句柄
		// param key 代表offset
		// param param 代表大小
		std::unordered_map<VkDeviceMemory, std::unordered_map<size_t, size_t>> m_image_empty_memory_ext;
		// param : offset
		std::unordered_map<VkDeviceMemory, size_t> m_image_offset_current_ext;

		/************************************************************************/
		// 统一内存块,预定义大小为glm::mat4 * 4096 * 4096
		const size_t PRE_BUFFER_MEMORY_SIZE = 4096 * 4096 * sizeof(glm::mat4);
		// 统一内存块，key: type bit
		std::unordered_map<uint32_t, VkDeviceMemory> m_buffer_memory;
		// bind后被释放的空闲的内存区间
		// key 代表内存句柄
		// param key 代表offset
		// param param 代表大小
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
		// 重建交换链信息
		SwapchainRecreateDetails*	p_swapchain_recreate_info = nullptr;
		// vulkan 内存分配器
		Vk_Allocator* p_allocator = nullptr;

	};
	extern ENGINE_API_THIRDPARTY VkManager vkManager;
}

