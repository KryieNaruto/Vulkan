#pragma once
#include <ThirdParty/vulkan/vulkan.h>
#include "vk_object.h"

#define GENERATE_BASIC_VKCLASS_FUNC(CLASS_NAME) \
public:\
    CLASS_NAME(const std::string& _label = #CLASS_NAME);\
    inline virtual ~CLASS_NAME() {\
        if(!p_createInfo) delete p_createInfo;\
        if(m_##CLASS_NAME != VK_NULL_HANDLE) vkDestroy##CLASS_NAME(m_vkInfo.m_device,m_##CLASS_NAME,m_vkInfo.p_allocate);\
    } \
    CLASS_NAME* recreate();\
    inline Vk##CLASS_NAME& getVk##CLASS_NAME() {return m_##CLASS_NAME;}\
private: \
    Vk##CLASS_NAME##CreateInfo* p_createInfo = nullptr; \
    Vk##CLASS_NAME m_##CLASS_NAME = VK_NULL_HANDLE;

namespace ThirdParty {

    class ENGINE_API_THIRDPARTY Semaphore :
        public vkObject
    {
        GENERATE_BASIC_VKCLASS_FUNC(Semaphore);
    };

}

