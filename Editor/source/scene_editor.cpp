#include "scene_editor.h"
#include "editor_global_func.h"
using namespace Editor::OnRenderFunc;

#define ICON_MESH_OPEN		"����.png"
#define ICON_MESH_CLOSE		"�����.png"
#define ICON_LIGHT_OPEN		"light.png"
#define ICON_LIGHT_CLOSE	"lightbulb_slash.png"
#define ICON_SKYBOX_OPEN	"material_converter-open.png"
#define ICON_SKYBOX_CLOSE	"material_converter-close.png"
#define ICON_SKYBOX_CONVOLUTION_OPEN	"�ۼƷ��ն�-open.png"
#define ICON_SKYBOX_CONVOLUTION_CLOSE	"�ۼƷ��ն�-close.png"

Editor::SceneEditor::SceneEditor()
{
	this->m_editor_title = "Scene";
	this->m_show = true;
}

Editor::SceneEditor::~SceneEditor()
{
	this->m_show = false;
	// ���ٸ�����
	auto _scene = p_scene->getRootScene()->getLast();
	// ��ȡβ�ڵ�
	while (_scene) {
		auto _temp_scene = _scene;
		_scene = _scene->getPrevious();
		delete _temp_scene;
	}
}

void Editor::SceneEditor::onInitialize()
{
	if (!m_initialized) {
		m_initialized = true;
		m_render_descriptor_sets.resize(vkInfo.m_frame_count);
	}
}

void Editor::SceneEditor::onStart()
{
	if(p_scene) {
		/************************************************************************/
		/* ���ڴ�С�ı�                                                         */
		/************************************************************************/
		if (m_render_resize && m_render_initialized) {
			p_scene->onResize(m_render_extent);
			auto p_scene_camera = p_scene->getMainCamera();
			for (size_t i = 0; i < vkInfo.m_frame_count; i++) {
				ImGui_ImplVulkan_RemoveTexture(m_render_descriptor_sets[i]);
				m_render_descriptor_sets[i] = ImGui_ImplVulkan_AddTexture(m_render_sampler->getVkSampler(), p_scene_camera->getVkImageView(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			m_render_resize = false;
			return;
		}
		// ��ʼ������ ��ʼ��descriptorSets
		if(!m_render_first && !m_render_initialized && p_scene->getMainCamera()->isReady()) {
			auto p_scene_camera = p_scene->getMainCamera();
			//-----------------------------------
			ThirdParty::Sampler* p_sampler = Core::Resource::RMaterial::getSampler(SAMPLER);
			const auto& vk_sampler = p_sampler->getVkSampler();
			m_render_sampler = p_sampler;
			//-----------------------------------
			for (size_t i = 0; i < vkInfo.m_frame_count; i++) {
				auto set = ImGui_ImplVulkan_AddTexture(vk_sampler, p_scene_camera->getVkImageView(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				m_render_descriptor_sets[i] = set;
			}
			//-----------------------------------
			//-----------------------------------
			m_render_initialized = true;
		}
	} else {
		p_scene = new Core::Resource::Scene(m_render_extent);
	}
}

void Editor::SceneEditor::onRender()
{
	if (m_initialized) {
		if(p_scene) Editor::OnRenderFunc::editor_scene_func(p_scene);
		// ��ʾScene����
		if (ImGui::Begin("Scene", &Editor::OnRenderFunc::editor_flags[Editor::OnRenderFunc::EDITOR_FLAGS::SCENE], ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)) {
			// ����Ƿ�Resize��
			checkOnResize();
			auto _window_pos = ImGui::GetWindowPos();
			if (p_scene) {
				p_scene->onUpdate();
				p_scene->onRender();
				auto p_scene_camera = p_scene->getMainCamera();
				// ��ʾС�ؼ�
				{
					// �Ƿ���ʾ2D����
					if (icon_check_box("Set 2D Mesh visualization", { ICON_MESH_CLOSE,ICON_MESH_OPEN }, p_scene->getShow_2d_mesh(),{20,20}));
					ImGui::SameLine();
					// �Ƿ���ʾӦ�ù���
					if (icon_check_box("Set Light visualization", { ICON_LIGHT_CLOSE,ICON_LIGHT_OPEN}, p_scene->getShow_light(),{20,20}));
					ImGui::SameLine();
					// �Ƿ���ʾӦ����պ�
					if (icon_check_box("Set Skybox visualization", { ICON_SKYBOX_CLOSE,ICON_SKYBOX_OPEN}, p_scene->getShow_skybox(),{20,20}));
					ImGui::SameLine(); ImGui::Checkbox("Debug", &p_scene->getIs_Debug()); ImGui::SameLine();
					// �Ƿ�ʹ�þ�����������պУ�
					if (icon_check_box("Use Skybox with Convolution", { ICON_SKYBOX_CONVOLUTION_CLOSE,ICON_SKYBOX_CONVOLUTION_OPEN }, p_scene->isUseConvolution(), { 20,20 }));
				}
				auto _next_pos_y = ImGui::GetCursorPosY();
				// Texture �������
				if (m_render_initialized && p_scene_camera->isReady()) {
					if (!p_scene_camera->isFirst()) {
						const auto& set = m_render_descriptor_sets[vkInfo.m_current_image];
						auto _padding_y = ImGui::GetStyle().FramePadding.y;
						m_render_size_current.y -= (_next_pos_y + 2 * _padding_y);
						ImVec2 uv0 = calcuUV0(m_render_size_current, m_render_extent);
						ImVec2 uv1 = calcuUV1(m_render_size_current, m_render_extent);
						if (m_render_size_current.x > m_render_extent.width || m_render_size_current.y > m_render_extent.height) {
							// �Ŵ�ͼ��
							ImVec2 size = { (float)m_render_extent.width ,(float)m_render_extent.height };
							// x > y, �Ŵ�ͼ��X���������
							if (m_render_size_current.x > m_render_size_current.y) {
								size.x = m_render_size_current.x;
								size.y = m_render_size_current.x / ((float)m_render_extent.width / (float)m_render_extent.height);
								// ����uv
								uv0 = calcuUV0(m_render_size_current, { (uint32_t)size.x,(uint32_t)size.y });
								uv1 = calcuUV1(m_render_size_current, { (uint32_t)size.x,(uint32_t)size.y });
							}
							else {
								size.y = m_render_size_current.y;
								size.x = m_render_size_current.y * ((float)m_render_extent.width / (float)m_render_extent.height);
								// ����uv
								uv0 = calcuUV0(m_render_size_current, { (uint32_t)size.x,(uint32_t)size.y });
								uv1 = calcuUV1(m_render_size_current, { (uint32_t)size.x,(uint32_t)size.y });
							}
						}
						auto _img_pos = ImGui::GetCursorPos();
						_img_pos.x += _window_pos.x;
						_img_pos.y += _window_pos.y;
						if (ImGui::BeginChild("RenderImage",{0,0},ImGuiChildFlags_None,ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
							// ͼ��
							ImGui::Image(set, { (float)m_render_size_current.x,(float)m_render_size_current.y }, uv0, uv1);
							if (ImGui::IsWindowHovered()) {
								// ����ʵ�ʻ�����С
								p_scene_camera->getProperty()->m_window_size = { (float)m_render_size_current.x,(float)m_render_size_current.y };
								p_scene_camera->getProperty()->m_window_pos = { (float)_img_pos.x,(float)_img_pos.y };
								p_scene_camera->focus();
							}
							else {
								p_scene_camera->unfocus();
							}
						}
						ImGui::EndChild();
						Editor::OnRenderFunc::drag_drop_file(SCENE, p_scene);
					}
				}
			}
		}
		ImGui::End();
	}
}

void Editor::SceneEditor::onResize()
{
	// ��һ����Ⱦ��ȡ���ڴ�С������
	if (m_render_first || m_render_resize)
	{
		// ��鴰�ڴ�С�Ƿ�ı�
		const auto& size = m_render_size_current;
		const auto& pos = m_render_pos_current;
		m_render_first = false;
		m_render_size = size;
		m_render_pos = pos;
		// ��ȡ��С
		m_render_extent.width = m_render_size.x - 2 * ImGui::GetStyle().FramePadding.x - 2 * ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetStyle().ScrollbarSize;
		m_render_extent.height = m_render_size.y - 2 * ImGui::GetStyle().FramePadding.y - 2 * ImGui::GetStyle().ItemInnerSpacing.y - ThirdParty::GetTabHeight("");

		m_render_extent = { 800,600, 1 };
	}
}

void Editor::SceneEditor::checkOnResize()
{
	// ���ڴ�С�ı���ֵΪ200px
	const int _size = 200;
	// ��鴰�ڴ�С�Ƿ�ı�
	m_render_size_current = ImGui::GetWindowSize();
	m_render_pos_current = ImGui::GetCursorPos();

	if (abs(m_render_size_current.x - m_render_size.x) > _size || abs(m_render_size_current.y - m_render_size.y) > _size) {
		m_render_resize = true;
	}

}

ImVec2 Editor::SceneEditor::calcuUV0(ImVec2 _canvas, const VkExtent3D& _tex_size)
{
	ImVec2 uv0 = { 0,0 };
	// x
	uv0.x = (_tex_size.width - _canvas.x) / 2 / _tex_size.width;
	uv0.x = uv0.x > 0 ? uv0.x : 0;
	// y
	uv0.y = (_tex_size.height - _canvas.y) / 2 / _tex_size.height;
	uv0.y = uv0.y > 0 ? uv0.y : 0;
	return uv0;
}

ImVec2 Editor::SceneEditor::calcuUV1(ImVec2 _canvas, const VkExtent3D& _tex_size)
{
	ImVec2 uv1 = { 1,1 };
	// x
	uv1.x = ((_tex_size.width - _canvas.x) / 2 + _canvas.x) / _tex_size.width;
	uv1.x = uv1.x > 0 ? uv1.x : 0;
	// y
	uv1.y = ((_tex_size.height - _canvas.y) / 2 + _canvas.y) / _tex_size.height;
	uv1.y = uv1.y > 0 ? uv1.y : 0;
	uv1.x = std::min(1.0f, uv1.x);
	uv1.y = std::min(1.0f, uv1.y);
	return uv1;
}
