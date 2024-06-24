#pragma once
#include <string>
#include "vk_object.h"
#include <vector>
namespace ThirdParty {
	// 队列，包含队列提交与呈现，不提供销毁和创建队列功能
	// 一个队列只包含一个SubmitInfo
	class ENGINE_API_THIRDPARTY Queue : public vkObject
	{
	public:
		Queue() = default;
		Queue(VkQueue _queue, const std::string& _label = "Queue");
		~Queue();
		// 提交
		VkResult submit(const VkFence& _fence = VK_NULL_HANDLE);
		// 呈现并更新帧索引, 只实现单交换链，不考虑多个交换链
		VkResult present();

		// 提交前设定
		Queue* setWaitSemaphores(const std::vector<VkSemaphore>& _waitSemaphores);
		Queue* setWaitPipelineStages(const std::vector<VkPipelineStageFlags>& _waitPipelineStages);
		Queue* setSignalSemaphores(const std::vector<VkSemaphore>& _signalSemaphores);
		Queue* setCommandBuffers(const std::vector<VkCommandBuffer>& _commandbuffers);
	private:
		// ================== 提交前必须设置阶段 ==================
		enum class SUBMIT_FLAG {
			WAIT_SEMAPHORE = 0x00,		// 等待信号
			WAIT_PIPELINE_STAGE = 0x03,	// 等待阶段
			SIGNAL_SEMAPHORE = 0x01,	// 目标信号
			COMMANDBUFFER = 0x02,		// 渲染命令
		};
		bool m_flag[16] = { false };
		// 临时储存对象
		std::vector<VkSemaphore> t_waitSemaphores;
		std::vector<VkPipelineStageFlags> t_waitPipelineStages;
		std::vector<VkSemaphore> t_signalSemaphores;
		std::vector<VkCommandBuffer> t_commandbuffers;
		VkFence t_fence;
		// ================== 呈现前必须设置阶段 ==================
		enum class PRESENT_FLAG {
			WAIT_SEMAPHORE = 0x00,
		};
		bool m_flag_2[16] = { false };



		// submitInfo 所需对象
		VkQueue m_queue = VK_NULL_HANDLE;
		VkSubmitInfo* p_submit_info = nullptr;
		VkPresentInfoKHR* p_present_info = nullptr;
	protected:
	};

}

