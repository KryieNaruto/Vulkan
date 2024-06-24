#include "vkobject/vk_queue.h"
#include <core/common/include/debug_to_console.h>
#include <vkParty/include/vk_util_function.h>

ThirdParty::Queue::Queue(VkQueue _queue, const std::string& _label /*= "Queue"*/)
	:vkObject(_label), m_queue(_queue)
{
	if (!this->p_submit_info) {
		this->p_submit_info = new VkSubmitInfo;
		p_submit_info->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		p_submit_info->pNext = nullptr;
	}
	if (!this->p_present_info) {
		this->p_present_info = new VkPresentInfoKHR;
		p_present_info->sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		p_present_info->pNext = nullptr;
	}
}

ThirdParty::Queue::~Queue()
{
	if (this->p_submit_info) delete this->p_submit_info;
	if (this->p_present_info) delete this->p_present_info;
}

VkResult ThirdParty::Queue::submit(const VkFence& _fence /*= VK_NULL_HANDLE*/)
{
	bool available = true;
	if (m_queue == VK_NULL_HANDLE) { Core::warn("Queue has not been set."); available &= false; }
	if (!m_flag[(uint16_t)SUBMIT_FLAG::WAIT_SEMAPHORE]) { Core::warn("[Queue Submit]Wait semaphores has not been set."); available &= false; }
	if (!m_flag[(uint16_t)SUBMIT_FLAG::WAIT_PIPELINE_STAGE]) { Core::warn("[Queue Submit]Wait pipeline stages has not been set."); available &= false; }
	if (!m_flag[(uint16_t)SUBMIT_FLAG::SIGNAL_SEMAPHORE]) { Core::warn("[Queue Submit]Signal semaphores has not been set."); available &= false; }
	if (!m_flag[(uint16_t)SUBMIT_FLAG::COMMANDBUFFER]) { Core::warn("[Queue Submit]CommandBuffer has not been set."); available &= false; }
	if (!available) return VK_ERROR_UNKNOWN;
	// WAIT
	p_submit_info->waitSemaphoreCount = static_cast<uint32_t>(t_waitSemaphores.size());
	p_submit_info->pWaitSemaphores = t_waitSemaphores.data();
	p_submit_info->pWaitDstStageMask = t_waitPipelineStages.data();
	// SIGNAL
	p_submit_info->signalSemaphoreCount = static_cast<uint32_t>(t_signalSemaphores.size());
	p_submit_info->pSignalSemaphores = t_signalSemaphores.data();
	// COMMAND BUFFER
	p_submit_info->commandBufferCount = static_cast<uint32_t>(t_commandbuffers.size());
	p_submit_info->pCommandBuffers = t_commandbuffers.data();

	// SUBMIT
	if (p_submit_info->commandBufferCount > 0)
		m_result = vkQueueSubmit(this->m_queue, 1, p_submit_info, _fence);
	memset(m_flag, 0, sizeof(m_flag));
	memset(m_flag_2, 0, sizeof(m_flag_2));
	return m_result;
}

VkResult ThirdParty::Queue::present()
{
	bool available = true;
	if (m_queue == VK_NULL_HANDLE) { Core::warn("Queue has not been set."); available &= false; }
	if (!m_flag_2[(uint16_t)PRESENT_FLAG::WAIT_SEMAPHORE]) { Core::warn("[Queue Present]Wait semaphores has not been set."); available &= false; }
	if (!available) return VK_ERROR_UNKNOWN;
	// WAIT
	p_present_info->waitSemaphoreCount = static_cast<uint32_t>(t_waitSemaphores.size());
	p_present_info->pWaitSemaphores = t_waitSemaphores.data();
	// SWAPCHAIN
	p_present_info->swapchainCount = 1;
	p_present_info->pSwapchains = &m_vkInfo.m_swapchain;
	p_present_info->pImageIndices = &m_vkInfo.m_current_image;
	// RESULT
	p_present_info->pResults = &m_result;

	m_result = vkQueuePresentKHR(this->m_queue, this->p_present_info);
	ThirdParty::checkSwapchainUsable(m_result);

	if (m_result == VK_SUCCESS) {
		// ¸üÐÂÖ¡Ë÷Òý
		m_vkInfo.nextFrame();
	}

	memset(m_flag, 0, sizeof(m_flag));
	memset(m_flag_2, 0, sizeof(m_flag_2));
	return m_result;
}

ThirdParty::Queue* ThirdParty::Queue::setWaitSemaphores(const std::vector<VkSemaphore>& _waitSemaphores)
{
	this->t_waitSemaphores = _waitSemaphores;
	this->m_flag[(uint16_t)SUBMIT_FLAG::WAIT_SEMAPHORE] = true;
	this->m_flag_2[(uint16_t)PRESENT_FLAG::WAIT_SEMAPHORE] = true;
	return this;
}

ThirdParty::Queue* ThirdParty::Queue::setWaitPipelineStages(const std::vector<VkPipelineStageFlags>& _waitPipelineStages)
{
	this->t_waitPipelineStages = _waitPipelineStages;
	this->m_flag[(uint16_t)SUBMIT_FLAG::WAIT_PIPELINE_STAGE] = true;
	return this;
}

ThirdParty::Queue* ThirdParty::Queue::setSignalSemaphores(const std::vector<VkSemaphore>& _signalSemaphores)
{
	this->t_signalSemaphores = _signalSemaphores;
	this->m_flag[(uint16_t)SUBMIT_FLAG::SIGNAL_SEMAPHORE] = true;
	return this;
}

ThirdParty::Queue* ThirdParty::Queue::setCommandBuffers(const std::vector<VkCommandBuffer>& _commandbuffers)
{
	this->t_commandbuffers = _commandbuffers;
	this->m_flag[(uint16_t)SUBMIT_FLAG::COMMANDBUFFER] = true;
	return this;
}
