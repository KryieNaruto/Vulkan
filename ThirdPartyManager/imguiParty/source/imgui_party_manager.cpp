#include "imgui_party_manager.h"
#include <core/common/include/debug_to_console.h>
#include <core/info/include/global_info.h>

ThirdParty::ImGuiManager::~ImGuiManager()
{
	if (this->p_init_info)
		delete this->p_init_info;
}

void ThirdParty::ImGuiManager::ImGuiCheckVkResult(VkResult result)
{
	if (result == VK_SUCCESS)
		return;
	ThirdParty::Core::error("imgui-vulkan has error");
	if (result < 0)
		abort();
}

void ThirdParty::ImGuiManager::onRenderStart()
{
	if (!this->m_bRenderEnd) {
		ThirdParty::Core::warn("The previous frame recording has not ended yet");
		return;
	}
	else {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		this->m_bRenderStart = true;
		this->m_bRendering = true;
		this->m_bRenderEnd = false;
	}
}

void ThirdParty::ImGuiManager::onRender()
{
	if (!this->m_bRendering) {
		ThirdParty::Core::warn("Frame recording not started");
		return;
	}
	else {
		ImGui::Render();
		this->p_draw_data = ImGui::GetDrawData();
	}
}

void ThirdParty::ImGuiManager::onRenderEnd()
{
	if (!this->m_bRenderStart) {
		ThirdParty::Core::warn("Frame recording not started");
		return;
	}
	else {
		this->m_bRenderStart = false;
		this->m_bRendering = false;
		this->m_bRenderEnd = true;
	}
	if (this->p_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void ThirdParty::ImGuiManager::render(VkCommandBuffer cmd)
{
	VkRenderPassBeginInfo renderBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderBeginInfo.renderPass = this->m_renderpass;
	renderBeginInfo.framebuffer = this->m_swapchain_framebuffers[ThirdParty::Core::g_ApplicationVulkanInfo.m_current_image];
	renderBeginInfo.renderArea.offset = { 0,0 };
	renderBeginInfo.renderArea.extent = ThirdParty::Core::g_ApplicationVulkanInfo.m_swapchain_extent;
	VkClearValue clearColor = { 0,0,0,1 };
	renderBeginInfo.clearValueCount = 1;
	renderBeginInfo.pClearValues = &clearColor;
	vkCmdBeginRenderPass(cmd, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	ImGui_ImplVulkan_RenderDrawData(this->p_draw_data, cmd);
	vkCmdEndRenderPass(cmd);
}

ThirdParty::ImGuiManager* ThirdParty::ImGuiManager::init()
{
	if (modeInfo.m_multiView) {
		createVulkanObject();
	}
	else {
		this->m_renderpass = vkInfo.m_renderpass;
		this->m_swapchain_image_views = vkInfo.m_swapchain_imageViews;
		this->m_swapchain_framebuffers = vkInfo.m_swapchain_framebuffers;
	}
	// 初始化imgui上下文
	IMGUI_CHECKVERSION();
	this->p_context = ImGui::CreateContext();
	this->p_io = &ImGui::GetIO();
	ImGuiIO& m_io_ref = *this->p_io; (void)m_io_ref;
	m_io_ref.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	m_io_ref.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	m_io_ref.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	m_io_ref.ConfigFlags |= modeInfo.m_multiView ? ImGuiConfigFlags_ViewportsEnable : 0x0;       // Enable Multi-Viewport / Platform Windows

	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();


	this->p_style = &ImGui::GetStyle();
	ImGuiStyle& m_style_ref = *this->p_style;
	if (m_io_ref.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		m_style_ref.WindowRounding = 0.0f;
		m_style_ref.Colors[ImGuiCol_WindowBg].w = 1.0;
	}
	m_style_ref.Colors[ImGuiCol_MenuBarBg] = ImColor(0.168f, 0.168f, 0.168f, 1.000f);
	m_style_ref.Colors[ImGuiCol_TitleBg] = ImColor(0.106f, 0.106f, 0.106f, 1.000f);

	// 初始化imgui_sdl_vulkan
	ImGui_ImplSDL2_InitForVulkan(sdlInfo.p_sdlwindow);
	this->p_init_info = new ImGui_ImplVulkan_InitInfo;
	p_init_info->Instance = vkInfo.m_instance;
	p_init_info->PhysicalDevice = vkInfo.m_physicalDevices[vkInfo.m_physicalIndex];
	p_init_info->Device = vkInfo.m_device;
	p_init_info->QueueFamily = vkInfo.m_present_queue_family_indices;
	p_init_info->Queue = vkInfo.m_present_queue;
	p_init_info->PipelineCache = VK_NULL_HANDLE;
	p_init_info->DescriptorPool = vkInfo.m_descriptor_pool;
	p_init_info->Subpass = 0;
	p_init_info->MinImageCount = vkInfo.m_frame_count;
	p_init_info->ImageCount = vkInfo.m_frame_count;
	p_init_info->MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	p_init_info->Allocator = vkInfo.p_allocate;
	p_init_info->UseDynamicRendering = false;
	p_init_info->MinAllocationSize = 0;
	p_init_info->CheckVkResultFn = ImGuiCheckVkResult;
	if (ImGui_ImplVulkan_Init(this->p_init_info, this->m_renderpass)) {
		// 加载字体
		auto ttf_path = ThirdParty::Core::getModuleCurrentDirectory();
		ttf_path.append("/resources/lang/SourceHanSans/SourceHanSansCN-Heavy.otf");
		this->p_io->Fonts->AddFontFromFileTTF(ttf_path.c_str(), 18.0f, NULL, this->p_io->Fonts->GetGlyphRangesChineseSimplifiedCommon());
		// 初始化字体
		ImGui_ImplVulkan_CreateFontsTexture();
		this->m_isInitialized = true;
	}
	if (this->m_isInitialized)
		ThirdParty::Core::success("ImGui_SDL2_Vulkan has been initialized");

	return this;
}

ThirdParty::ImGuiManager* ThirdParty::ImGuiManager::getManager()
{
	return this;
}

void ThirdParty::ImGuiManager::destroy()
{
	vkDeviceWaitIdle(ThirdParty::Core::g_ApplicationVulkanInfo.m_device);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	destroyVulkanObject();
}

void ThirdParty::ImGuiManager::windowResize()
{
	destroyVulkanObject();
	if (modeInfo.m_multiView) {
		createVulkanObject();
	}
	else {
		this->m_renderpass = vkInfo.m_renderpass;
		this->m_swapchain_image_views = vkInfo.m_swapchain_imageViews;
		this->m_swapchain_framebuffers = vkInfo.m_swapchain_framebuffers;
	}
	this->m_isInitialized = true;
}

void ThirdParty::ImGuiManager::createVulkanObject()
{
	// 初始化imgui renderpass
	{
		VkAttachmentDescription attachment = {};
		attachment.format = vkInfo.m_surface_format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;
		VkResult result = vkCreateRenderPass(vkInfo.m_device, &info, vkInfo.p_allocate, &this->m_renderpass);
		ImGuiCheckVkResult(result);
	}
	// 创建imageview
	{
		for (const auto& swapchainImage : vkInfo.m_swapchain_images) {
			VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.image = swapchainImage;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = Core::g_ApplicationVulkanInfo.m_surface_format;
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
			viewInfo.pNext = nullptr;
			VkImageView view;
			VkResult result = vkCreateImageView(vkInfo.m_device, &viewInfo, vkInfo.p_allocate, &view);
			ImGuiCheckVkResult(result);
			this->m_swapchain_image_views.push_back(view);
		}
	}
	// 初始化imgui framebuffer
	{
		this->m_swapchain_framebuffers.resize(vkInfo.m_frame_count);
		for (size_t i = 0; i < this->m_swapchain_framebuffers.size(); i++) {
			std::vector<VkImageView> attachments = {
				this->m_swapchain_image_views[i],
			};

			VkFramebufferCreateInfo frameInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			frameInfo.renderPass = this->m_renderpass;
			frameInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameInfo.pAttachments = attachments.data();
			frameInfo.width = appInfo.m_window_width;
			frameInfo.height = appInfo.m_window_height;
			frameInfo.layers = 1;
			VkResult result = vkCreateFramebuffer(vkInfo.m_device, &frameInfo, vkInfo.p_allocate, &this->m_swapchain_framebuffers[i]);
			ImGuiCheckVkResult(result);
		}
	}
}

void ThirdParty::ImGuiManager::destroyVulkanObject()
{
	if (modeInfo.m_multiView) {
		for (size_t i = 0; i < vkInfo.m_frame_count; i++) {
			vkDestroyImageView(vkInfo.m_device, this->m_swapchain_image_views[i], vkInfo.p_allocate);
			vkDestroyFramebuffer(vkInfo.m_device, this->m_swapchain_framebuffers[i], vkInfo.p_allocate);
		}
		if (this->m_renderpass != VK_NULL_HANDLE)
			vkDestroyRenderPass(ThirdParty::Core::g_ApplicationVulkanInfo.m_device, this->m_renderpass, ThirdParty::Core::g_ApplicationVulkanInfo.p_allocate);
	}
}
