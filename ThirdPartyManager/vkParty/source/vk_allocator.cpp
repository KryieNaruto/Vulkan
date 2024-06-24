#include "vk_allocator.h"
#include <stdlib.h>

ThirdParty::Vk_Allocator::~Vk_Allocator()
{
	if (this->allocationCallback)
		delete this->allocationCallback;
}

VkAllocationCallbacks* ThirdParty::Vk_Allocator::getAllocator()
{
	if (this->allocationCallback)
		return this->allocationCallback;
	this->allocationCallback = new VkAllocationCallbacks();
	this->allocationCallback->pUserData = (void*)this;
	this->allocationCallback->pfnAllocation = reinterpret_cast<PFN_vkAllocationFunction>(&allocationFunction);
	this->allocationCallback->pfnFree = reinterpret_cast<PFN_vkFreeFunction>(&freeFunction);
	this->allocationCallback->pfnReallocation = reinterpret_cast<PFN_vkReallocationFunction>(&reallocationFunction);
	this->allocationCallback->pfnInternalAllocation = PFN_vkInternalAllocationNotification(&internalAllocationNotification);
	this->allocationCallback->pfnInternalFree = PFN_vkInternalFreeNotification(&internalFreeNotification);
	return this->allocationCallback;
}

void* ThirdParty::Vk_Allocator::allocationFunction(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return malloc(size);
}

void ThirdParty::Vk_Allocator::freeFunction(void* pUserData, void* pMemory)
{

}

void* ThirdParty::Vk_Allocator::reallocationFunction(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return realloc(pOriginal, size);
}

void ThirdParty::Vk_Allocator::internalAllocationNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{

}

void ThirdParty::Vk_Allocator::internalFreeNotification(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{

}
