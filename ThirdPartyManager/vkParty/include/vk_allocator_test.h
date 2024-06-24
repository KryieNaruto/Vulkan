#pragma once
#include "vk_allocator.h"


namespace ThirdParty {
    class Vk_Allocator_Test :
        public Vk_Allocator
    {
    public:
        virtual VkAllocationCallbacks* getAllocator() override;
    };
}

