#pragma once
#include "RRenderPass.h"
#include "Shader.h"
#include "RMaterial.h"
#include "model/RShape.h"

namespace Core::Resource {

	// Ԥ�����ʲ� 
	// VkAttachmentDescription
	/*
	VkAttachmentDescriptionFlags    flags
	VkFormat                        format
	VkSampleCountFlagBits           samples
	VkAttachmentLoadOp              loadOp
	VkAttachmentStoreOp             storeOp
	VkAttachmentLoadOp              stencilLoadOp
	VkAttachmentStoreOp             stencilStoreOp
	VkImageLayout                   initialLayout
	VkImageLayout                   finalLayout
	*/
	extern VkAttachmentDescription g_attachment_color;

	extern VkAttachmentDescription g_attachment_depth_stencil;

	void preload_sampler();
	void preload_renderpass();
	void preload_shape();

	// Ԥ������Դ
	ENGINE_API_RESOURCE void preload();
}