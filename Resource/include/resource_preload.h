#pragma once
#include "RRenderPass.h"
#include "Shader.h"
#include "RMaterial.h"
#include "model/RShape.h"

namespace Core::Resource {

	// 预定义资产 
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

	// 预加载资源
	ENGINE_API_RESOURCE void preload();
}