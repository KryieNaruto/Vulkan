#pragma once
#include <ThirdParty/vulkan/vulkan.h>
namespace ThirdParty {
	//===========================
	// vulkan 内存分配器
	//===========================
	class Vk_Allocator
	{
	public:
		Vk_Allocator() = default;
		virtual ~Vk_Allocator();

		virtual VkAllocationCallbacks* getAllocator() = 0;
	private:

		static void* allocationFunction(
			void* pUserData,
			size_t                      size,
			size_t                      alignment,
			VkSystemAllocationScope     allocationScope);

		static void freeFunction(
			void* pUserData,
			void* pMemory);

		//************************************
		// Method:    reallocationFunction
		// FullName:  ThirdParty::Vk_Allocator::reallocationFunction
		// Access:    private 
		// Returns:   void*
		// Qualifier: 返回主机内存地址
		// Parameter: void * pUserData
		// Parameter: void * pOriginal
		// Parameter: size_t size
		// Parameter: size_t alignment
		// Parameter: VkSystemAllocationScope allocationScope
		//************************************
		static void* reallocationFunction(
			void* pUserData,
			void* pOriginal,
			size_t                      size,
			size_t                      alignment,
			VkSystemAllocationScope     allocationScope);


		static void internalAllocationNotification(
			void* pUserData,
			size_t                      size,
			VkInternalAllocationType    allocationType,
			VkSystemAllocationScope     allocationScope);

		static void internalFreeNotification(
			void* pUserData,
			size_t                      size,
			VkInternalAllocationType    allocationType,
			VkSystemAllocationScope     allocationScope);

		VkAllocationCallbacks* allocationCallback = nullptr;
	protected:
	};
}

