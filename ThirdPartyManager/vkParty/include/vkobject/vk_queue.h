#pragma once
#include <string>
#include "vk_object.h"
#include <vector>
namespace ThirdParty {
	// ���У����������ύ����֣����ṩ���ٺʹ������й���
	// һ������ֻ����һ��SubmitInfo
	class ENGINE_API_THIRDPARTY Queue : public vkObject
	{
	public:
		Queue() = default;
		Queue(VkQueue _queue, const std::string& _label = "Queue");
		~Queue();
		// �ύ
		VkResult submit(const VkFence& _fence = VK_NULL_HANDLE);
		// ���ֲ�����֡����, ֻʵ�ֵ��������������Ƕ��������
		VkResult present();

		// �ύǰ�趨
		Queue* setWaitSemaphores(const std::vector<VkSemaphore>& _waitSemaphores);
		Queue* setWaitPipelineStages(const std::vector<VkPipelineStageFlags>& _waitPipelineStages);
		Queue* setSignalSemaphores(const std::vector<VkSemaphore>& _signalSemaphores);
		Queue* setCommandBuffers(const std::vector<VkCommandBuffer>& _commandbuffers);
	private:
		// ================== �ύǰ�������ý׶� ==================
		enum class SUBMIT_FLAG {
			WAIT_SEMAPHORE = 0x00,		// �ȴ��ź�
			WAIT_PIPELINE_STAGE = 0x03,	// �ȴ��׶�
			SIGNAL_SEMAPHORE = 0x01,	// Ŀ���ź�
			COMMANDBUFFER = 0x02,		// ��Ⱦ����
		};
		bool m_flag[16] = { false };
		// ��ʱ�������
		std::vector<VkSemaphore> t_waitSemaphores;
		std::vector<VkPipelineStageFlags> t_waitPipelineStages;
		std::vector<VkSemaphore> t_signalSemaphores;
		std::vector<VkCommandBuffer> t_commandbuffers;
		VkFence t_fence;
		// ================== ����ǰ�������ý׶� ==================
		enum class PRESENT_FLAG {
			WAIT_SEMAPHORE = 0x00,
		};
		bool m_flag_2[16] = { false };



		// submitInfo �������
		VkQueue m_queue = VK_NULL_HANDLE;
		VkSubmitInfo* p_submit_info = nullptr;
		VkPresentInfoKHR* p_present_info = nullptr;
	protected:
	};

}

