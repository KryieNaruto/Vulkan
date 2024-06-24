#include "vk_party_manager.h"
#include "vk_allocator_test.h"
#include "vk_util_function.h"
#include "core/info/include/global_info.h"
#include "core/common/include/debug_to_console.h"
#include "assimp/vector2.h"
#include <ThirdParty/vulkan/vulkan.h>
#include <assert.h>
#include <SDL2/SDL_vulkan.h>
#include <set>
#include <fstream>
#include <filesystem>
#include <cstring>

using namespace ThirdParty;
using namespace Core;
ENGINE_API_THIRDPARTY ThirdParty::VkManager ThirdParty::vkManager;

VkManager::~VkManager()
{

}

uint32_t& VkManager::getCurrentSwapchainImageIndex()
{
	vkWaitForFences(m_device, 1, &m_fences[ThirdParty::Core::g_ApplicationVulkanInfo.m_current_frame], VK_TRUE, UINT32_MAX);
	vkResetFences(m_device, 1, &m_fences[ThirdParty::Core::g_ApplicationVulkanInfo.m_current_frame]);
	this->result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT32_MAX, m_image_available_semaphores[Core::g_ApplicationVulkanInfo.m_current_frame], VK_NULL_HANDLE, &Core::g_ApplicationVulkanInfo.m_current_image);
	// ��齻����
	checkSwapchainUsable(result);
	return Core::g_ApplicationVulkanInfo.m_current_image;
}

uint32_t& VkManager::getCurrentFrameIndex()
{
	return Core::g_ApplicationVulkanInfo.m_current_frame;
}

void VkManager::destroyShaderModule(VkShaderModule& _module)
{
	vkDestroyShaderModule(m_device, _module, p_allocator->getAllocator());
	_module = VK_NULL_HANDLE;
}

void VkManager::bindImageMemory(VkImage _image, size_t& _size, size_t& _offset)
{
	// ��_image������С�ڴ�
	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(m_device, _image, &memReq);
	// ��req.size �� req.aligment����С������
	int factor = ceil((float)memReq.size / (float)memReq.alignment);
	size_t size = (factor)*memReq.alignment;
	// memReq��alignment����ͼ���С���ı��
	size_t offset = m_image_offset_current;
	// ��֤offset��alignment�ı���
	if (floor((float)offset / (float)memReq.alignment) != ceil((float)offset / (float)memReq.alignment)) {
		// ����С���Ա�aligment��������
		auto _fator = offset / memReq.alignment + 1;
		offset = _fator * memReq.alignment;
		m_image_offset_current = offset;
	}
	// ���Ȳ��ҿ��еĵ�ַ
	for (auto& available : m_image_empty_memory) {
		if (available.second >= size) {
			_offset = available.first;
			_size = size;
			// ��֤offset��alignment�ı���
			if (floor((float)_offset / (float)memReq.alignment) != ceil((float)_offset / (float)memReq.alignment)) {
				// ����С���Ա�aligment��������
				auto _fator = _offset / memReq.alignment + 1;
				offset = _fator * memReq.alignment;
			}
			vkBindImageMemory(m_device, _image, m_image_memory, offset);
			size_t new_offset = available.first + size;
			size_t new_size = available.second - size;
			if (new_size != 0)
				m_image_empty_memory.insert(std::make_pair(new_offset, new_size));
			m_image_empty_memory.erase(available.first);
			return;
		}
	}
	vkBindImageMemory(m_device, _image, m_image_memory, m_image_offset_current);
	_offset = m_image_offset_current;
	_size = size;
	m_image_offset_current += size;
}

VkCommandBuffer VkManager::BeginSingleCommandBuffer()
{
	if (m_command_buffer_temp == VK_NULL_HANDLE) {
		VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = m_graphics_command_pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkAllocateCommandBuffers(m_device, &allocInfo, &m_command_buffer_temp);
	}
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	if (m_fence_temp == VK_NULL_HANDLE) {
		VkFenceCreateInfo createInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(m_device, &createInfo, p_allocator->getAllocator(), &m_fence_temp);
	}
	result = vkWaitForFences(m_device, 1, &m_fence_temp, VK_TRUE, UINT32_MAX);
	if (checkVkResult(result, "Fence Time Wait Out")) {
		vkResetFences(m_device, 1, &m_fence_temp);
		vkBeginCommandBuffer(m_command_buffer_temp, &beginInfo);
	}
	return m_command_buffer_temp;
}

void VkManager::endSingleCommandBuffer()
{
	if (result == VK_SUCCESS) {
		vkEndCommandBuffer(m_command_buffer_temp);
		VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &m_command_buffer_temp;

		vkQueueSubmit(m_graphics_queue, 1, &submit, m_fence_temp);
	}
}

void VkManager::unbindImageMemory(size_t size, size_t offset)
{
	m_image_empty_memory.insert(std::make_pair(offset, size));
}

VkShaderModule VkManager::createShaderModule(const std::vector<uint32_t> _binaries)
{
	// ����shaderModule
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	info.codeSize = _binaries.size() * sizeof(uint32_t);
	info.pCode = reinterpret_cast<const uint32_t*>(_binaries.data());
	if (checkVkResult(vkCreateShaderModule(m_device, &info, p_allocator->getAllocator(), &shaderModule), "Failed to create shader module"))
	{
		Core::info("Success to create Shader Module from memory %p", &_binaries[0]);
		return shaderModule;
	}
	return VK_NULL_HANDLE;
}

VkDescriptorSetLayout VkManager::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& _createInfo)
{
	VkDescriptorSetLayout _b;
	result = vkCreateDescriptorSetLayout(m_device, &_createInfo, p_allocator->getAllocator(), &_b);
	if (checkVkResult(result, "Failed to create VkDescriptorSetLayout.")) {
		Core::info("Success to create VkDescriptorSetLayout.");
		return _b;
	}
	return VK_NULL_HANDLE;
}

VkPipelineLayout VkManager::createPipelineLayout(const VkPipelineLayoutCreateInfo& _createInfo)
{
	VkPipelineLayout _b;
	result = vkCreatePipelineLayout(m_device, &_createInfo, p_allocator->getAllocator(), &_b);
	if (checkVkResult(result, "Failed to create VkPipelineLayout.")) {
		Core::info("Success to create VkPipelineLayout.");
		return _b;
	}
	return VK_NULL_HANDLE;
}

VkRenderPass VkManager::createRenderPass(const VkRenderPassCreateInfo& _c)
{
	VkRenderPass _b;
	result = vkCreateRenderPass(m_device, &_c, p_allocator->getAllocator(), &_b);
	if (checkVkResult(result, "Failed to create VkRenderPass.")) {
		Core::info("Success to create VkRenderPass.");
		return _b;
	}
	return VK_NULL_HANDLE;
}

void VkManager::destroyRenderPass(VkRenderPass& _r)
{
	vkDestroyRenderPass(m_device, _r, p_allocator->getAllocator());
	_r = VK_NULL_HANDLE;
}

void VkManager::createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop, VkBuffer& _buffer, VkDeviceMemory& _mem, const std::vector<uint32_t> _queues)
{
	// ����Buffer
	VkBufferCreateInfo _c = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	_c.pNext = 0;
	_c.flags = 0;
	_c.size = _size;
	_c.usage = _usage;
	_c.queueFamilyIndexCount = _queues.size();
	_c.pQueueFamilyIndices = _queues.data();
	_c.sharingMode = _queues.empty() ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
	vkCreateBuffer(m_device, &_c, p_allocator->getAllocator(), &_buffer);

	// �����ڴ�
	_mem = createBufferMemory(_buffer, _prop);
}

void VkManager::destroyBuffer(VkBuffer& _buffer)
{
	vkDestroyBuffer(m_device, _buffer, p_allocator->getAllocator());
	_buffer = VK_NULL_HANDLE;
}

VkDeviceMemory VkManager::createBufferMemory(VkBuffer _buffer, VkMemoryPropertyFlags _prop)
{
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(m_device, _buffer, &req);

	VkDeviceMemory mem;
	VkMemoryAllocateInfo _alloc = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	_alloc.allocationSize = req.size;
	_alloc.memoryTypeIndex = findMemoryType(req.memoryTypeBits, _prop);

	// ��������ڣ��򴴽������򷵻����е�
	if (m_buffer_memory.find(_alloc.memoryTypeIndex) == m_buffer_memory.end()) {
		vkAllocateMemory(m_device, &_alloc, p_allocator->getAllocator(), &mem);
		m_buffer_memory[_alloc.memoryTypeIndex] = mem;
		m_buffer_offset_current[mem] = 0;
		m_buffer_empty_memory[mem] = {};
	}
	else mem = m_buffer_memory[_alloc.memoryTypeIndex];

	return mem;
}

void VkManager::destroyBufferMemory(VkDeviceMemory _mem)
{
	vkFreeMemory(m_device, _mem, p_allocator->getAllocator());
}

VkBuffer VkManager::createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _prop, const std::vector<uint32_t> _queues /*= {}*/)
{
	VkBuffer _buffer;
	VkBufferCreateInfo _c = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	_c.pNext = 0;
	_c.flags = 0;
	_c.size = _size;
	_c.usage = _usage;
	_c.queueFamilyIndexCount = _queues.size();
	_c.pQueueFamilyIndices = _queues.data();
	_c.sharingMode = _queues.empty() ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
	vkCreateBuffer(m_device, &_c, p_allocator->getAllocator(), &_buffer);
	return _buffer;
}

VkDeviceMemory VkManager::bindBufferMemory(VkBuffer _buffer, VkMemoryPropertyFlags _prop, size_t& _size, size_t& _offset)
{
	VkMemoryRequirements _memReq;
	vkGetBufferMemoryRequirements(m_device, _buffer, &_memReq);
	_size = _memReq.size;
	// ��ȡ���õ�memory,���û�У��򷵻ز����������ʾ
	uint32_t typebit = findMemoryType(_memReq.memoryTypeBits, _prop);
	const auto& _mem_ite = m_buffer_memory.find(typebit);
	if (_mem_ite == m_buffer_memory.end()) {
		Core::error("Failed to find suitable memory:%p", _buffer);
		return VK_NULL_HANDLE;
	}
	// �п���mem
	auto& _mem = _mem_ite->second;
	// ���Ȳ��ҿ�������
	{
		auto& _mem_empty = m_buffer_empty_memory[_mem];
		for (auto& _mem_rect : _mem_empty) {
			auto& __offset = _mem_rect.first;
			auto& __size = _mem_rect.second;
			// ����������������������,������ʹ�ÿ������򲢼����µĿ�������
			if (__size >= _size) {
				_offset = __offset;
				// ƫ����������aligment ��
				if (ceil((float)__offset / (float)_memReq.alignment) != floor((float)__offset / (float)_memReq.alignment)) {
					// ���ﲻӦ������2�ߵ���С������.���Ǳ�_offset��΢��һ��Ŀ��Ա�req.aligment��������
					auto _factor = __offset / _memReq.alignment + 1; // ��һ��ƫ����
					auto _offset_current_temp = _factor * _memReq.alignment;
					// ����size
					auto _size_temp = _size + _offset_current_temp - __offset;
					_offset = _offset_current_temp;
					// �ж���size�Ƿ�С���ṩ��__size
					if (__size < _size_temp) continue;
					// ��ȡ��ʵ��С
					_size = _memReq.size;
				}
				vkBindBufferMemory(m_device, _buffer, _mem, _offset);
				// ����������
				__size = __size - _size;
				// ���������Ϊ0�����޳���ǰƫ�� ������
				if (__size == 0) {
					_mem_empty.erase(__offset);
					return _mem;
				}
				else {
					// ������ƫ��
					auto new_offset = _offset + _size;
					// ���������,�Ƴ�������
					_mem_empty[new_offset] = __size;
					_mem_empty.erase(__offset);
					return _mem;
				}
			}
		}
	}

	auto& _mem_offset_current = m_buffer_offset_current[_mem];	// ��ȡ��ǰ�ڴ�ƫ��
	// ƫ����������aligment ��
	if (ceil((float)_mem_offset_current / (float)_memReq.alignment) != floor((float)_mem_offset_current / (float)_memReq.alignment)) {
		// ���ﲻӦ������2�ߵ���С������.���Ǳ�_offset��΢��һ��Ŀ��Ա�req.aligment��������
		auto _factor = _mem_offset_current / _memReq.alignment + 1; // ��һ��ƫ����
		auto _offset_current_temp = _factor * _memReq.alignment;
		// ����size
		_size += _offset_current_temp - _mem_offset_current;
		_mem_offset_current = _offset_current_temp;
	}
	vkBindBufferMemory(m_device, _buffer, _mem, _mem_offset_current);
	// ��ȡ��ʵ��С
	_size = _memReq.size;
	// ����ƫ��
	_offset = _mem_offset_current;
	_mem_offset_current += _size;

	return _mem;
}

void VkManager::unbindBufferMemory(VkDeviceMemory _mem, size_t _size, size_t _offset)
{
	const auto& _offset_empty_ite = m_buffer_empty_memory.find(_mem);
	if (_offset_empty_ite == m_buffer_empty_memory.end()) {
		Core::error("Failed to find memory:%p", _mem);
		return;
	}
	auto& _offset_empty = _offset_empty_ite->second;
	_offset_empty[_offset] = _size;
}

void VkManager::bindBufferData(const VkDeviceMemory& _mem, void* _data, size_t _offset, size_t _size)
{
	void* data;
	vkMapMemory(m_device, _mem, _offset, _size, 0, &data);
	memcpy(data, _data, _size);
	vkUnmapMemory(m_device, _mem);
}

std::variant<VkImage, VkImageView, VkDeviceMemory>
VkManager::createVkObject(std::variant<
	VkImageCreateInfo*,
	VkImageViewCreateInfo*,
	VkMemoryAllocateInfo*
> _ci)
{
	switch (_ci.index())
	{
	case 0:
	{
		VkImage _i;
		if (checkVkResult(
			vkCreateImage(m_device, std::get<VkImageCreateInfo*>(_ci), p_allocator->getAllocator(), &_i),
			"Failed to create VkImage!"))
			return _i;
		else return (VkImage)VK_NULL_HANDLE;
	}
	case 1:
	{
		VkImageView _i;
		if (checkVkResult(
			vkCreateImageView(m_device, std::get<VkImageViewCreateInfo*>(_ci), p_allocator->getAllocator(), &_i),
			"Failed to create VkImageView!"))
			return _i;
		else return (VkImageView)VK_NULL_HANDLE;
	}
	case 2:
	{
		VkDeviceMemory _i;
		if (checkVkResult(
			vkAllocateMemory(m_device, std::get<VkMemoryAllocateInfo*>(_ci), p_allocator->getAllocator(), &_i),
			"Failed to create VkDeviceMemory!"))
			return _i;
		else return (VkDeviceMemory)VK_NULL_HANDLE;
	}
	default:
		break;
	}
}

void VkManager::destroyVkObject(std::variant<VkImage, VkImageView, VkPipeline, VkPipelineLayout,VkDescriptorSetLayout> _o)
{
	switch (_o.index())
	{
	case 0:
		vkDestroyImage(m_device, std::get<VkImage>(_o), p_allocator->getAllocator());
		break;
	case 1:
		vkDestroyImageView(m_device, std::get<VkImageView>(_o), p_allocator->getAllocator());
		break;
	case 2:
		vkDestroyPipeline(m_device, std::get<VkPipeline>(_o), p_allocator->getAllocator());
		break;
	case 3:
		vkDestroyPipelineLayout(m_device, std::get<VkPipelineLayout>(_o), p_allocator->getAllocator());
		break;
	case 4:
		vkDestroyDescriptorSetLayout(m_device, std::get<VkDescriptorSetLayout>(_o), p_allocator->getAllocator());
	default:
		break;
	}
}

VkDeviceMemory VkManager::bindImageMemory(VkImage _image, VkMemoryPropertyFlags _prop, size_t& size, size_t& offset)
{
	VkMemoryRequirements _req;
	vkGetImageMemoryRequirements(m_device, _image, &_req);
	size = _req.size;	// ��ȡsize
	// �Ȳ����Ƿ���ڷ���Ҫ����ڴ�飬�������򴴽�
	VkDeviceMemory _mem;
	uint32_t typebit = findMemoryType(_req.memoryTypeBits, _prop);
	const auto& _mem_ite = m_image_memory_ext.find(typebit);
	// ������
	if (_mem_ite == m_image_memory_ext.end()) {
		VkMemoryAllocateInfo _alloc = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		_alloc.allocationSize = PRE_IMAGE_MEMORY_SIZE_EXT;
		_alloc.memoryTypeIndex = typebit;
		_alloc.pNext = 0;
		_mem = std::get<VkDeviceMemory>(vkManager.createVkObject(&_alloc));
		m_image_memory_ext[typebit] = _mem;	// ����mem
		m_image_empty_memory_ext[_mem] = {};// ��ʼ��empty
		m_image_offset_current_ext[_mem] = 0;// ��ʼ��current
	}
	else _mem = _mem_ite->second;


	// �Ȳ��ҿ�������, ����ʹ����С����
	auto& _empty_memory_ite = m_image_empty_memory_ext[_mem];
	std::map<size_t, size_t, std::greater<size_t>> _empty_size_offset;
	for (auto _empty : _empty_memory_ite) {
		// �������sizeС�ڵ��ڿ��������򽫿�������ƫ�����С����
		if (size <= _empty.second) {
			auto _offset = _empty.first;
			auto _size = size;
			// ���ƫ���Ƿ����
			// ����ƫ�Ʒ���req.aligment ������
			if (ceil((float)_offset / (float)_req.alignment) != floor((float)_offset / (float)_req.alignment)) {
				// ���ﲻӦ������2�ߵ���С������.���Ǳ�_offset��΢��һ��Ŀ��Ա�req.aligment��������
				auto _factor = _offset / _req.alignment + 1; // ��һ��ƫ����
				auto _offset_current_temp = _factor * _req.alignment;
				// ����size
				_size += _offset_current_temp - _offset;
				_offset = _offset_current_temp;
				// �������size��_empty��С, �����������size�����ϣ��򲻱���
				if (_empty.second < _size) continue;
			}

			_empty_size_offset.insert({ size,  _empty.first });
		}
	}
	// ʹ����С����
	if (!_empty_size_offset.empty()) {
		// map����
		auto _rect = _empty_size_offset.begin();
		offset = _rect->second;	// ����memƫ��
		// ����ƫ�Ʒ���req.aligment ������
		if (ceil((float)offset / (float)_req.alignment) != floor((float)offset / (float)_req.alignment)) {
			// ���ﲻӦ������2�ߵ���С������.���Ǳ�_offset��΢��һ��Ŀ��Ա�req.aligment��������
			auto _factor = offset / _req.alignment + 1; // ��һ��ƫ����
			auto _offset_current_temp = _factor * _req.alignment;
			// ����size
			size += _offset_current_temp - offset;
			offset = _offset_current_temp;
		}
		vkBindImageMemory(m_device, _image, _mem, offset);
		// �����µ�����size��offset
		size_t new_size = _rect->first - size;
		size_t new_offset = _rect->second + size;
		// ������Ǹոպã����µ��������empty
		if (new_size != 0) {
			m_image_empty_memory_ext[_mem][new_offset] = new_size;
		}
		// ����ǰ�����empty��ȥ��
		m_image_empty_memory_ext[_mem].erase(_rect->second);
		return _mem;
	}

	// ���û�п�����������µ�����
	// ��ȡ��ǰmemƫ��
	auto& _offset_current = m_image_offset_current_ext[_mem];
	// ����ƫ�Ʒ���req.aligment ������
	if (ceil((float)_offset_current / (float)_req.alignment) != floor((float)_offset_current / (float)_req.alignment)) {
		// ���ﲻӦ������2�ߵ���С������.���Ǳ�_offset��΢��һ��Ŀ��Ա�req.aligment��������
		auto _factor = _offset_current / _req.alignment + 1; // ��һ��ƫ����
		auto _offset_current_temp = _factor * _req.alignment;
		// ����size
		size += _offset_current_temp - _offset_current;
		_offset_current = _offset_current_temp;
	}

	// ����memorySize - offset�Ƿ���ϴ�С
	auto _empty_size = PRE_IMAGE_MEMORY_SIZE_EXT - _offset_current;
	if (_empty_size < size) {
		Core::error("Image memory is not enough:empty[%d]:need[%d]", _empty_size, size);
		return _mem;
	}

	vkBindImageMemory(m_device, _image, _mem, _offset_current);
	// ����ƫ��
	offset = _offset_current;
	// ���µ�ǰ�ڴ�ƫ��
	_offset_current += size;

	return _mem;
}

void VkManager::unbindImageMemory(VkDeviceMemory& _mem, size_t size, size_t offset)
{
	m_image_empty_memory_ext[_mem][offset] = size;
}

void VkManager::generateMipmaps(VkImage _image, const VkExtent3D& _size, uint32_t mipLevels, uint32_t _layers)
{
	auto cmd = BeginSingleCommandBuffer();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = _image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,_layers,1 };

	int mipWidth = _size.width;
	int mipHeight = _size.height;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr,
			1, &barrier);

		VkImageBlit blit;
		blit.srcOffsets[0] = { 0,0,0 };
		blit.srcOffsets[1] = { mipWidth,mipHeight,1 };
		blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT,i - 1,_layers,1 };
		blit.dstOffsets[0] = { 0,0,0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,mipHeight > 1 ? mipHeight / 2 : 1,1 };
		blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT,i,_layers,1 };

		vkCmdBlitImage(
			cmd,
			_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr,
			1, &barrier);
		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	// translate���һ��
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr,
		1, &barrier);

	endSingleCommandBuffer();
}

void VkManager::endSingleCommandBuffer(VkCommandBuffer _cmd)
{
	if (result == VK_SUCCESS) {
		vkEndCommandBuffer(_cmd);
		VkSubmitInfo submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &_cmd;

		vkQueueSubmit(m_graphics_queue, 1, &submit, VK_NULL_HANDLE);
	}
	// ����cmd
	vkDeviceWaitIdle(m_device);
	vkFreeCommandBuffers(m_device, m_graphics_command_pool, 1, &_cmd);
}

VkCommandBuffer VkManager::BeginNewSigleCommandBuffer()
{
	VkCommandBuffer _cmd = VK_NULL_HANDLE;
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = m_graphics_command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(m_device, &allocInfo, &_cmd);

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(_cmd, &beginInfo);
	return _cmd;
};

VkManager* ThirdParty::VkManager::init()
{
	PRE_IMAGE_MEMORY_SIZE_EXT = 4096 * 4096 * 4 * 4 * 2;
	ThirdParty::Core::ApplicationVulkanInfo& vkInfo = ThirdParty::Core::g_ApplicationVulkanInfo;
	ThirdParty::Core::ApplicationWindowInfo& appInfo = ThirdParty::Core::g_ApplicationWindowInfo;
	// ��ʼ��ָ���Ա����
	{
		p_swapchain_recreate_info = new ThirdParty::SwapchainRecreateDetails;
	}
	// ��ʼ��������
	{
		this->p_allocator = new Vk_Allocator_Test();
		Core::g_ApplicationVulkanInfo.p_allocate = p_allocator->getAllocator();
	}
	// ��ʼ��vkinstance
	{
		VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.apiVersion = VK_API_VERSION_1_3;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pApplicationName = nullptr;
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "ZsEngine";
		appInfo.pNext = nullptr;

		VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		instanceInfo.pApplicationInfo = &appInfo;
		// bindless	��dynamic renderpass
		vkInfo.m_instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(Core::g_ApplicationVulkanInfo.m_instance_extensions.size());
		instanceInfo.ppEnabledExtensionNames = Core::g_ApplicationVulkanInfo.m_instance_extensions.data();
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(Core::g_ApplicationVulkanInfo.m_validation_layers.size());
		instanceInfo.ppEnabledLayerNames = Core::g_ApplicationVulkanInfo.m_validation_layers.data();
		instanceInfo.pNext = nullptr;

		result = vkCreateInstance(&instanceInfo, this->p_allocator->getAllocator(), &this->m_instance);
		if (checkVkResult(result, "Failed to create VkInstance!")) {
			// ����instance��Ϣ��ȫ��
			Core::g_ApplicationVulkanInfo.m_instance = this->m_instance;
			Core::info("VkInstance has been created");
		}
	}
	// ��ʼ�������
	if (Core::g_ApplicationInfo.m_debug) {
		VkDebugUtilsMessengerCreateInfoEXT debugInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		debugInfo.messageSeverity = 0x00000001 | 0x00000010 | 0x00000100 | 0x00001000;
		debugInfo.messageType = 0x00000001 | 0x00000002 | 0x00000004 | 0x00000008;
		debugInfo.pfnUserCallback = ThirdParty::debugCallback;
		debugInfo.pUserData = nullptr;

		result = ThirdParty::createDebugUtilsMessengerEXT(m_instance, &debugInfo, this->p_allocator->getAllocator(), &this->callback);
		checkVkResult(result, "Failed to create debug callback");
	}
	// �������ڱ���
	{
		if (SDL_Vulkan_CreateSurface(Core::g_ApplicationSDLInfo.p_sdlwindow, this->m_instance, &this->m_surface) == SDL_TRUE) {
			Core::g_ApplicationVulkanInfo.m_surface = this->m_surface;
			Core::info("VkSurfaceKHR has been created");
		}
	}
	// ��ȡ�����豸
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(this->m_instance, &deviceCount, 0);
		Core::g_ApplicationVulkanInfo.m_physicalDevices.resize(deviceCount);
		vkEnumeratePhysicalDevices(this->m_instance, &deviceCount, Core::g_ApplicationVulkanInfo.m_physicalDevices.data());
		if (deviceCount == 0) {
			Core::error("Failed to find GPU which is support Vulkan");
			assert(deviceCount);
		}
		this->m_physical_device = Core::g_ApplicationVulkanInfo.m_physicalDevices[Core::g_ApplicationVulkanInfo.m_physicalIndex];
		// ��ȡ��������
		vkInfo.m_sample_count = getMaxUsableSampleCount();
	}
	// �����߼��豸&����
	{
		QueueFamilyIndices indices = findQueueFamilies(Core::g_ApplicationVulkanInfo.m_physicalDevices[Core::g_ApplicationVulkanInfo.m_physicalIndex]);
		// ��������
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> uniqueQueueFamilies = { indices.graphicsFamily,indices.presentFamily };
		float queuePriority = 1.0f;
		for (int queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueInfo.queueFamilyIndex = queueFamily;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;
			queueInfo.pNext = nullptr;
			queueCreateInfos.push_back(queueInfo);
		}
		// �豸����
		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.geometryShader = VK_TRUE;
		// bindless
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
		vkInfo.m_bindless = checkBindless();
		if (vkInfo.m_bindless) {
			indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
			indexingFeatures.pNext = nullptr;
			indexingFeatures.runtimeDescriptorArray = VK_TRUE;
			indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
			indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
			indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

			Core::g_ApplicationVulkanInfo.m_device_extensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
			// descriptor indexing ��Ҫinstance��property2��maintenance2
			Core::g_ApplicationVulkanInfo.m_device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
			//Core::g_ApplicationVulkanInfo.m_device_extensions.push_back(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
			Core::info("Bindless: supported");
		}
		else
			Core::warn("Bindless: unsupported");
		//---------------------------------------------------------
		// dynamic renderpass (���ʱ༭������Ҫ�����,����������Framebuffer RenderPass ȥ����Ⱦ������)
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderFratures{};
		vkInfo.m_dynamic_renderpass = checkDeviceExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
		if (vkInfo.m_dynamic_renderpass) {
			// dynamic Rendering ��Ҫ VK_KHR_depth_stencil_resolve
			Core::g_ApplicationVulkanInfo.m_device_extensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
			Core::g_ApplicationVulkanInfo.m_device_extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
			dynamicRenderFratures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
			dynamicRenderFratures.pNext = nullptr;
			dynamicRenderFratures.dynamicRendering = VK_TRUE;
			Core::info("Dynamic RenderPass: supported");
			indexingFeatures.pNext = &dynamicRenderFratures;
		}
		else {
			Core::warn("Dynamic RenderPass: unsupported;");
		}
		// �����������豸��չ
		Core::g_ApplicationVulkanInfo.m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		// ���֧��
		ThirdParty::checkDeviceExtensionSupport(Core::g_ApplicationVulkanInfo.m_physicalDevices[Core::g_ApplicationVulkanInfo.m_physicalIndex]);
		// �����豸
		VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceInfo.pNext = &indexingFeatures;
		deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceInfo.pQueueCreateInfos = queueCreateInfos.data();

		deviceInfo.pEnabledFeatures = &deviceFeatures;
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(Core::g_ApplicationVulkanInfo.m_device_extensions.size());
		deviceInfo.ppEnabledExtensionNames = Core::g_ApplicationVulkanInfo.m_device_extensions.data();
		deviceInfo.enabledLayerCount = static_cast<uint32_t>(Core::g_ApplicationVulkanInfo.m_validation_layers.size());
		deviceInfo.ppEnabledLayerNames = Core::g_ApplicationVulkanInfo.m_validation_layers.data();

		result = vkCreateDevice(Core::g_ApplicationVulkanInfo.m_physicalDevices[Core::g_ApplicationVulkanInfo.m_physicalIndex], &deviceInfo, this->p_allocator->getAllocator(), &this->m_device);
		if (checkVkResult(result, "Failed to create VkDevice!")) {
			Core::g_ApplicationVulkanInfo.m_device = this->m_device;
			// ��ȡ����
			vkGetDeviceQueue(this->m_device, indices.graphicsFamily, 0, &this->m_graphics_queue);
			vkGetDeviceQueue(this->m_device, indices.graphicsFamily, 0, &this->m_present_queue);
			Core::g_ApplicationVulkanInfo.m_graphics_queue_family_indices = indices.graphicsFamily;
			Core::g_ApplicationVulkanInfo.m_present_queue_family_indices = indices.presentFamily;
			Core::g_ApplicationVulkanInfo.m_graphics_queue = this->m_graphics_queue;
			Core::g_ApplicationVulkanInfo.m_present_queue = this->m_present_queue;
			Core::info("VkDevice & VkQueue[Graphics|Present] has been created");
		}
	}
	// ����swapchain
	{
		// ������ϸ��
		SwapChainSupportDetails details = querySwapChainSupport(Core::g_ApplicationVulkanInfo.m_physicalDevices[Core::g_ApplicationVulkanInfo.m_physicalIndex]);
		auto surfaceFormat = chooseSurfaceFormatKHR(details.formats);
		Core::g_ApplicationVulkanInfo.m_surface_format = surfaceFormat.format;
		Core::g_ApplicationVulkanInfo.m_surface_color_space = surfaceFormat.colorSpace;
		Core::g_ApplicationVulkanInfo.m_surface_present_mode = choosePresentModeKHR(details.presentModes);
		Core::g_ApplicationVulkanInfo.m_swapchain_extent = chooseSwapExtent(details.capabilities);
		Core::g_ApplicationVulkanInfo.m_frame_count = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && Core::g_ApplicationVulkanInfo.m_frame_count > details.capabilities.maxImageCount)
			Core::g_ApplicationVulkanInfo.m_frame_count = details.capabilities.maxImageCount;
		VkSwapchainCreateInfoKHR& swapchainInfo = this->p_swapchain_recreate_info->m_swapchainInfo;
		swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapchainInfo.surface = Core::g_ApplicationVulkanInfo.m_surface;
		swapchainInfo.minImageCount = Core::g_ApplicationVulkanInfo.m_frame_count;
		swapchainInfo.imageFormat = Core::g_ApplicationVulkanInfo.m_surface_format;
		swapchainInfo.imageColorSpace = Core::g_ApplicationVulkanInfo.m_surface_color_space;
		swapchainInfo.imageExtent = Core::g_ApplicationVulkanInfo.m_swapchain_extent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		// �����
		QueueFamilyIndices indices = findQueueFamilies(Core::g_ApplicationVulkanInfo.m_physicalDevices[Core::g_ApplicationVulkanInfo.m_physicalIndex]);
		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily,(uint32_t)indices.presentFamily };
		if (indices.graphicsFamily != indices.presentFamily) {
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainInfo.queueFamilyIndexCount = 2;
			swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainInfo.queueFamilyIndexCount = 0;
			swapchainInfo.pQueueFamilyIndices = nullptr;
		}
		swapchainInfo.preTransform = details.capabilities.currentTransform;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// �Ƿ���л��
		swapchainInfo.presentMode = Core::g_ApplicationVulkanInfo.m_surface_present_mode;
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

		result = vkCreateSwapchainKHR(this->m_device, &swapchainInfo, this->p_allocator->getAllocator(), &this->m_swapchain);
		if (checkVkResult(result, "Failed to create VkSwapchainKHR")) {
			Core::g_ApplicationVulkanInfo.m_swapchain = this->m_swapchain;
			Core::info("VkSwapchainKHR has been created");
		}
		// ��ȡ������ͼ��
		vkGetSwapchainImagesKHR(this->m_device, this->m_swapchain, &Core::g_ApplicationVulkanInfo.m_frame_count, 0);
		this->m_swapchainImages.resize(Core::g_ApplicationVulkanInfo.m_frame_count);
		vkGetSwapchainImagesKHR(this->m_device, this->m_swapchain, &Core::g_ApplicationVulkanInfo.m_frame_count, this->m_swapchainImages.data());
		// ����������ͼ����ͼ
		ThirdParty::createSwapchainImageViews(vkInfo.m_surface_format, this->m_swapchainImages, this->m_swapchain_imageViews);
		Core::g_ApplicationVulkanInfo.m_swapchain_images = this->m_swapchainImages;
		Core::g_ApplicationVulkanInfo.m_swapchain_imageViews = this->m_swapchain_imageViews;
	}
	// ����RenderPass
	{
		VkAttachmentDescription attachment = {};
		attachment.format = vkInfo.m_surface_format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;
		this->result = vkCreateRenderPass(vkInfo.m_device, &info, vkInfo.p_allocate, &this->m_renderpass);
		if (checkVkResult(result, "Failed to create basic VkRenderpass")) {
			vkInfo.m_renderpass = this->m_renderpass;
			Core::info("VkRenderpass has been created");
		}
	}
	// ����֡����
	{
		this->m_swapchain_framebuffers.resize(Core::g_ApplicationVulkanInfo.m_frame_count);
		for (size_t i = 0; i < this->m_swapchain_framebuffers.size(); i++) {
			std::vector<VkImageView> attachments = {
				this->m_swapchain_imageViews[i],
			};

			VkFramebufferCreateInfo frameInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			frameInfo.renderPass = vkInfo.m_renderpass;
			frameInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameInfo.pAttachments = attachments.data();
			frameInfo.width = Core::vkInfo.m_swapchain_extent.width;
			frameInfo.height = Core::vkInfo.m_swapchain_extent.height;
			frameInfo.layers = 1;
			result = vkCreateFramebuffer(this->m_device, &frameInfo, vkInfo.p_allocate, &this->m_swapchain_framebuffers[i]);
			if (checkVkResult(result, "Failed to create basic VkFramebuffer")) {
				vkInfo.m_swapchain_framebuffers.push_back(this->m_swapchain_framebuffers[i]);
				Core::info("VkFramebuffer has been created");
			}
		}
	}
	// ����ͬ������
	{
		this->m_image_available_semaphores.resize(Core::g_ApplicationVulkanInfo.m_frame_count);
		this->m_render_finished_semaphores.resize(Core::g_ApplicationVulkanInfo.m_frame_count);
		this->m_fences.resize(Core::g_ApplicationVulkanInfo.m_frame_count);
		bool success = true;
		for (size_t i = 0; i < Core::g_ApplicationVulkanInfo.m_frame_count; i++) {
			VkSemaphoreCreateInfo semInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			semInfo.pNext = nullptr;
			semInfo.flags = 0;
			result = vkCreateSemaphore(this->m_device, &semInfo, this->p_allocator->getAllocator(), &this->m_image_available_semaphores[i]);
			success &= checkVkResult(result, "Failed to create VkSemaphore[Image Available]");
			result = vkCreateSemaphore(this->m_device, &semInfo, this->p_allocator->getAllocator(), &this->m_render_finished_semaphores[i]);
			success &= checkVkResult(result, "Failed to create VkSemaphore[Render Finished]");
			VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			fenceInfo.pNext = nullptr;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			result = vkCreateFence(this->m_device, &fenceInfo, this->p_allocator->getAllocator(), &this->m_fences[i]);
			success &= checkVkResult(result, "Failed to create VkFence");
		}
		if (success) {
			vkInfo.m_image_available_semaphores = this->m_image_available_semaphores;
			vkInfo.m_render_finished_semaphores = this->m_render_finished_semaphores;
			vkInfo.m_fences = this->m_fences;
			Core::info("Main synchronous has been created");
		}
	}
	// ����descriptor pool
	{
		std::vector<VkDescriptorPoolSize> pool_sizes =
		{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1024},
			{VK_DESCRIPTOR_TYPE_SAMPLER,1024},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,1024},
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		pool_info.maxSets = 1024;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		result = vkCreateDescriptorPool(vkInfo.m_device, &pool_info, vkInfo.p_allocate, &this->m_descriptor_pool);
		if (checkVkResult(result, "Failed to create VkDescriptorPool")) {
			vkInfo.m_descriptor_pool = this->m_descriptor_pool;
			Core::info("VkDescriptorPool has been created");
		}
	}
	// ������graphics command pool
	{
		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = vkInfo.m_graphics_queue_family_indices;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		result = vkCreateCommandPool(this->m_device, &poolInfo, this->p_allocator->getAllocator(), &this->m_graphics_command_pool);
		if (checkVkResult(result, "Failed to create VkCommandPool[TRANSIENT]")) {
			vkInfo.m_graphics_command_pool = this->m_graphics_command_pool;
			Core::info("VkCommandPool[TRANSIENT] has been created");
		}
	}
	// �������������
	{
		vkInfo.m_main_command_buffers.resize(Core::g_ApplicationVulkanInfo.m_frame_count);

		VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocInfo.commandPool = vkInfo.m_graphics_command_pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = Core::g_ApplicationVulkanInfo.m_frame_count;
		result = vkAllocateCommandBuffers(this->m_device, &allocInfo, vkInfo.m_main_command_buffers.data());
		if (checkVkResult(result, "Failed to create VkCommandBuffer[Main]")) {
			Core::info("VkCommandBuffer[Main] has been created");
		}
	}
	// ������������Сͼ���豸�ڴ�
	{
		VkImage _i;
		{
			VkImageCreateInfo _c = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
			_c.arrayLayers = 0;
			_c.extent = { vkInfo.m_swapchain_extent.width,vkInfo.m_swapchain_extent.height,1 };
			_c.format = vkInfo.m_surface_format;
			_c.imageType = VK_IMAGE_TYPE_2D;
			_c.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			_c.mipLevels = 1;
			_c.arrayLayers = 1;
			_c.pQueueFamilyIndices = 0;
			_c.queueFamilyIndexCount = 0;
			_c.samples = VK_SAMPLE_COUNT_1_BIT;
			_c.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			_c.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			_c.tiling = VK_IMAGE_TILING_OPTIMAL;
			vkCreateImage(m_device, &_c, p_allocator->getAllocator(), &_i);
		}

		VkMemoryRequirements req;
		vkGetImageMemoryRequirements(m_device, _i, &req);
		size_t size = req.size * this->PRE_IMAGE_MEMORY_SIZE;
		m_image_memory = createImageDeviceMemory(size, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (m_image_memory != VK_NULL_HANDLE)
			Core::info("VkDeviceMemory[Image Format:R8G8B8A8_UNORM] has been created");

		vkDestroyImage(m_device, _i, p_allocator->getAllocator());
	}
	// ��������ҪBuffer������
	{
		VkBuffer _buffer;
		// ���㻺��
		{
			VkDeviceMemory _memory;
			createBuffer(
				PRE_BUFFER_MEMORY_SIZE,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _buffer, _memory);
			destroyBuffer(_buffer);
		}
		// �ݴ滺��
		{
			VkDeviceMemory _memory;
			createBuffer(
				PRE_BUFFER_MEMORY_SIZE,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _buffer, _memory);
			destroyBuffer(_buffer);
		}
		// UniformBuffer����
		{
			VkDeviceMemory _memory;
			createBuffer(
				PRE_BUFFER_MEMORY_SIZE,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _buffer, _memory);
			destroyBuffer(_buffer);
		}
		// Index buffer
		{
			VkDeviceMemory _memory;
			createBuffer(
				PRE_BUFFER_MEMORY_SIZE,
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _buffer, _memory);
			destroyBuffer(_buffer);
		}
	}

	this->m_isInitialized = true;
	if (this->m_isInitialized == true) {
		Core::success("Vulkan has been initialized");
	}
	return this;
}

VkManager* ThirdParty::VkManager::getManager()
{
	return this;
}

void ThirdParty::VkManager::destroy()
{
	if (p_swapchain_recreate_info)
		delete p_swapchain_recreate_info;
	vkDeviceWaitIdle(this->m_device);
	if (m_image_memory != VK_NULL_HANDLE) vkFreeMemory(m_device, m_image_memory, p_allocator->getAllocator());
	for (const auto& _mem : m_buffer_memory)
		if (_mem.second != VK_NULL_HANDLE) vkFreeMemory(m_device, _mem.second, p_allocator->getAllocator());
	for (const auto& _mem : m_image_memory_ext)
		if (_mem.second != VK_NULL_HANDLE) vkFreeMemory(m_device, _mem.second, p_allocator->getAllocator());
	if (m_fence_temp != VK_NULL_HANDLE) vkDestroyFence(m_device, m_fence_temp, p_allocator->getAllocator());
	if (m_transfer_command_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(m_device, m_transfer_command_pool, this->p_allocator->getAllocator());
	if (m_graphics_command_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(m_device, m_graphics_command_pool, this->p_allocator->getAllocator());
	if (m_descriptor_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(m_device, m_descriptor_pool, this->p_allocator->getAllocator());
	if (m_renderpass != VK_NULL_HANDLE)
		vkDestroyRenderPass(this->m_device, this->m_renderpass, this->p_allocator->getAllocator());
	for (uint32_t i = 0; i < Core::g_ApplicationVulkanInfo.m_frame_count; i++) {
		vkDestroyImageView(this->m_device, this->m_swapchain_imageViews[i], this->p_allocator->getAllocator());
		vkDestroySemaphore(this->m_device, this->m_image_available_semaphores[i], this->p_allocator->getAllocator());
		vkDestroySemaphore(this->m_device, this->m_render_finished_semaphores[i], this->p_allocator->getAllocator());
		vkDestroyFence(this->m_device, this->m_fences[i], this->p_allocator->getAllocator());
		vkDestroyFramebuffer(this->m_device, this->m_swapchain_framebuffers[i], this->p_allocator->getAllocator());
	}
	if (m_swapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(this->m_device, this->m_swapchain, this->p_allocator->getAllocator());
	if (m_surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(this->m_instance, this->m_surface, this->p_allocator->getAllocator());
	if (m_device != VK_NULL_HANDLE)
		vkDestroyDevice(this->m_device, this->p_allocator->getAllocator());
	if (callback != VK_NULL_HANDLE)
		ThirdParty::destroyDebugUtilsMessengerEXT(m_instance, callback, this->p_allocator->getAllocator());
	if (m_instance != VK_NULL_HANDLE)
		vkDestroyInstance(this->m_instance, this->p_allocator->getAllocator());
	if (this->p_allocator)
		delete p_allocator;
}

VkShaderModule VkManager::createShaderModule(const std::string _path)
{
	if (!std::filesystem::exists(std::filesystem::path(_path))) return VK_NULL_HANDLE;
	std::ifstream file(_path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		warn(("Failed to load shader file:" + _path).c_str());
		return VK_NULL_HANDLE;
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	// ����shaderModule
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	info.codeSize = fileSize;
	info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
	if (checkVkResult(vkCreateShaderModule(m_device, &info, p_allocator->getAllocator(), &shaderModule), "Failed to create shader module"))
	{
		Core::info(("Shader Module has been created:" + _path).c_str());
		return shaderModule;
	}
	else return VK_NULL_HANDLE;
}

void VkManager::windowResize()
{
	// �ȴ�����
	vkQueueWaitIdle(this->m_present_queue);
	vkQueueWaitIdle(this->m_graphics_queue);
	// �ȴ��豸
	vkDeviceWaitIdle(ThirdParty::Core::vkInfo.m_device);
	// ���ٵ�ǰ������ͼ����ͼ/֡����
	{
		for (uint32_t i = 0; i < Core::g_ApplicationVulkanInfo.m_frame_count; i++) {
			vkDestroyImageView(this->m_device, this->m_swapchain_imageViews[i], this->p_allocator->getAllocator());
			vkDestroyFramebuffer(this->m_device, this->m_swapchain_framebuffers[i], this->p_allocator->getAllocator());
		}
		vkDestroySwapchainKHR(this->m_device, this->m_swapchain, this->p_allocator->getAllocator());
	}
	// �����ź���
	{
		vkDestroySemaphore(m_device, m_image_available_semaphores[vkInfo.m_current_frame], p_allocator->getAllocator());
		vkDestroySemaphore(m_device, m_render_finished_semaphores[vkInfo.m_current_frame], p_allocator->getAllocator());
		vkDestroyFence(m_device, m_fences[vkInfo.m_current_frame], p_allocator->getAllocator());
	}
	uint32_t _result = VK_SUCCESS;
	// �ؽ�������
	{
		// ��ȡ�µ�extent��Χ
		VkExtent2D& newExtent = ThirdParty::Core::vkInfo.m_swapchain_extent;
		int w, h;
		SDL_Vulkan_GetDrawableSize(Core::sdlInfo.p_sdlwindow, &w, &h); newExtent = { (uint32_t)w,(uint32_t)h };
		this->p_swapchain_recreate_info->m_swapchainInfo.imageExtent = newExtent;
		_result |= vkCreateSwapchainKHR(Core::vkInfo.m_device, &this->p_swapchain_recreate_info->m_swapchainInfo, Core::vkInfo.p_allocate, &this->m_swapchain);
		if (!_result) {
			Core::vkInfo.m_swapchain = this->m_swapchain;
			Core::info("Swapchain has been recreated");
		}
		// ��ȡ������ͼ��
		if (!this->m_swapchainImages.empty()) this->m_swapchainImages.clear();
		vkGetSwapchainImagesKHR(this->m_device, this->m_swapchain, &Core::g_ApplicationVulkanInfo.m_frame_count, 0);
		this->m_swapchainImages.resize(Core::g_ApplicationVulkanInfo.m_frame_count);
		vkGetSwapchainImagesKHR(this->m_device, this->m_swapchain, &Core::g_ApplicationVulkanInfo.m_frame_count, this->m_swapchainImages.data());
		// ����������ͼ����ͼ
		ThirdParty::createSwapchainImageViews(vkInfo.m_surface_format, this->m_swapchainImages, this->m_swapchain_imageViews);
		Core::g_ApplicationVulkanInfo.m_swapchain_images = this->m_swapchainImages;
		Core::g_ApplicationVulkanInfo.m_swapchain_imageViews = this->m_swapchain_imageViews;
	}
	// �ؽ�֡����
	{
		{
			if (!this->m_swapchain_framebuffers.empty()) this->m_swapchain_framebuffers.clear();
			this->m_swapchain_framebuffers.resize(Core::g_ApplicationVulkanInfo.m_frame_count);
		}
		for (size_t i = 0; i < this->m_swapchain_framebuffers.size(); i++) {
			// framebuffer 
			{
				std::vector<VkImageView> attachments = {
					this->m_swapchain_imageViews[i],
				};
				VkFramebufferCreateInfo frameInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
				frameInfo.renderPass = vkInfo.m_renderpass;
				frameInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				frameInfo.pAttachments = attachments.data();
				frameInfo.width = Core::vkInfo.m_swapchain_extent.width;
				frameInfo.height = Core::vkInfo.m_swapchain_extent.height;
				frameInfo.layers = 1;
				_result |= vkCreateFramebuffer(this->m_device, &frameInfo, vkInfo.p_allocate, &this->m_swapchain_framebuffers[i]);
			}
		}
		if (!_result) {
			vkInfo.m_swapchain_framebuffers = this->m_swapchain_framebuffers;
			Core::info("Swapchain framebuffers have been recreated");
		}
	}
	// �ؽ��ź���
	{
		VkSemaphoreCreateInfo semInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		semInfo.pNext = nullptr;
		semInfo.flags = 0;
		VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		vkCreateSemaphore(m_device, &semInfo, p_allocator->getAllocator(), &m_image_available_semaphores[vkInfo.m_current_frame]);
		vkCreateSemaphore(m_device, &semInfo, p_allocator->getAllocator(), &m_render_finished_semaphores[vkInfo.m_current_frame]);
		vkCreateFence(m_device, &fenceInfo, p_allocator->getAllocator(), &m_fences[vkInfo.m_current_frame]);

		vkInfo.m_image_available_semaphores = m_image_available_semaphores;
		vkInfo.m_render_finished_semaphores = m_render_finished_semaphores;
		vkInfo.m_fences = m_fences;
	}

	if (!_result) {
		vkResetFences(this->m_device, 1, &this->m_fences[Core::vkInfo.m_current_frame]);
		this->m_isInitialized = true;
		Core::success("Swapchain has been recreated");
	}
}

