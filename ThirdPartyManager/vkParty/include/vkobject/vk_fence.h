#pragma once
#include "vk_object.h"

namespace ThirdParty {
    class ENGINE_API_THIRDPARTY Fence:
        public vkObject
    {
    public:
        Fence(const std::string& _label = "Fence");
        virtual ~Fence();
        inline VkFence& getVkFence() { return m_fence; }
        Fence* recreate();
    private:
        VkFence m_fence = VK_NULL_HANDLE;
        VkFenceCreateInfo* p_createInfo = nullptr;
    };

}

