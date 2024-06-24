#pragma once
#include "vk_object.h"
namespace ThirdParty {
	
	/************************************************************************/
	/* VkSubpass descriptionµÄ·â×°											*/
	/************************************************************************/

	class ENGINE_API_THIRDPARTY Subpass
		: public vkObject
	{
	public: 
		Subpass(const std::string& _label = "Subpass");
		virtual ~Subpass();

		VkSubpassDescription& getVkSubpassDescription();

		Subpass* addColorAttachmentRef(uint32_t _att_index, VkImageLayout _layout);
		Subpass* addInputAttachmentRef(uint32_t _att_index, VkImageLayout _layout);
		Subpass* addResolveAttachmentRef(uint32_t _att_index, VkImageLayout _layout);
		Subpass* addDepthStencilAttachmentRef(uint32_t _att_index, VkImageLayout _layout);

	private:
		std::vector<VkAttachmentReference> m_attachment_input;
		std::vector<VkAttachmentReference> m_attachment_color;
		std::vector<VkAttachmentReference> m_attachment_resolve;
		std::vector<VkAttachmentReference> m_attachment_depth_stencil;

		VkSubpassDescription* p_description = nullptr;
	};

}

