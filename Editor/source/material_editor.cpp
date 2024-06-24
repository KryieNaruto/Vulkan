#define IMGUI_DEFINE_MATH_OPERATORS
#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_internal.h>
#include "material_editor.h"

Editor::MaterialEditor::MaterialEditor()
{
	m_editor_title = "Material Editor";
	m_show = true;
	p_options = new imgui::Options;
	{
		p_options->m_select_border_color = IM_COL32(0, 248, 255, 200);
		p_options->m_quad_selection = IM_COL32(0, 248, 255, 200);
		p_options->m_quad_selection_border = IM_COL32(0, 248, 255, 64);
	}
	p_view_state = new imgui::ViewState;
	p_material_delegate = new MaterialDelegate;
	p_material_delegate->g_delegate = p_material_delegate;
}

Editor::MaterialEditor::~MaterialEditor()
{
	if (p_options) delete p_options;
	if (p_view_state) delete p_view_state;
	if (p_material_delegate) delete p_material_delegate;
}

void Editor::MaterialEditor::onRender()
{
	onInput();
	if (ImGui::Begin("Material Editor Debug")) {
		if (ImGui::CollapsingHeader("Material Editor Inner Debug")) {
			ImGui::Text("Loading File:%s", m_file_path_loading.c_str());
		}

		if (ImGui::CollapsingHeader("Link Debug")) {
			auto _d = p_view_state->m_debug_info;
			ImGui::Text("Last Select Node:%d", _d.m_select_node_index);
			ImGui::Text("Hover Node:%d", _d.m_hover_node_index);
			ImGui::Text("Hover Slot:%d", _d.m_hover_slot_index);

			if (ImGui::CollapsingHeader("Links")) {
				for (const auto& _link : p_material_delegate->p_links) {
					ImGui::Text("Input Node:%d", _link->m_input_node_index);
					ImGui::Text("Input Slot:%d", _link->m_input_slot_index);
					ImGui::Text("Output Node:%d", _link->m_output_node_index);
					ImGui::Text("Output Slot:%d", _link->m_output_slot_index);
					ImGui::Separator();
				}
			}
		}
	}
	ImGui::End();
	if (ImGui::Begin(this->m_editor_title.c_str())) {
		// Add
		handleEditor();
		handleLoading();
		// Material Editor
		{
			p_options->m_canvas_start_pos = ImGui::GetCursorScreenPos();
			if (imgui::beginCanvas(*p_view_state, *p_options)) {
				if (p_material_delegate) {
					for (uint32_t j = 0; j < 2; j++) {
						// 绘制节点
						for (size_t _node_index = 0; _node_index < p_material_delegate->getNodeCount(); _node_index++) {
							auto _node = p_material_delegate->getNode(_node_index);
							if (_node->m_select != (j != 0)) continue;
							imgui::beginNode(*p_view_state, *p_options, *p_material_delegate, _node_index);
							{
								auto _template = p_material_delegate->getTemplate(_node->m_template_index);
								for (size_t i = 0; i < _template->p_outputs.size(); i++) {
									if (imgui::beginSlotTemplate(_node, _node_index, _template, _template->p_outputs, i)) {
										// Tempalte中的内容在SlotTemplate的m_func中定义，其为lambda
										imgui::endSlotTemplate();
									}
								}
								for (size_t i = 0; i < _template->p_inputs.size(); i++) {
									if (imgui::beginSlotTemplate(_node, _node_index, _template, _template->p_inputs, i)) {
										imgui::endSlotTemplate();
									}
								}
							}
							imgui::endNode();
						}
					}
				}
			}
			ThirdParty::imgui::endCanvas();
			// 拖放文件
			m_file_path_loading = Editor::OnRenderFunc::drag_drop_file(OnRenderFunc::MATERIAL_EDITOR, nullptr);
		}
		m_is_active = true;
	}
	else {
		m_is_active = false;	// 止步于此了 2024年5月28日22:28:16
	}
	ImGui::End();
	if (m_is_active) {
		// 绘制预览图
		for (auto _cam_ite : p_cameras_createting) {
			auto _cam = _cam_ite.second;
			auto _mat = _cam_ite.first;
			// 链接mat的preview 与cam的color attachment
			if (_cam->isReady() && _mat->isReady()) {
				if (_mat->m_preview.empty()) {
					for (int i = 0; i < vkInfo.m_frame_count; i++) {
						auto _des = ImGui_ImplVulkan_AddTexture(RMaterial::getSampler()->getVkSampler(), _cam->getVkImageView(i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						_mat->m_preview.push_back(_des);
					}
				}
				_cam->onRenderStart();
				auto _obj = Core::Resource::getShape(SPHERE, "Material Editor Preview Sphere");
				auto _func = p_scene->getDrawFunc(_mat->getShader());
				_obj
					->setRenderCamera(_cam)
					->setMaterial(_mat)
					->draw(_cam->getVkCommandBuffer(), _func);
				_cam->onRenderEnd();
			}
		}
	}
}

void Editor::MaterialEditor::onUpdate()
{
	p_scene = Editor::OnRenderFunc::p_current_scene;
}

void Editor::MaterialEditor::onInput()
{
	const auto GIO = ImGui::GetIO();
	// 控制Options /ViewState
	// 中键平移
	if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && p_view_state->m_is_active) {
		m_is_active_detail = false;
		p_view_state->m_position += GIO.MouseDelta / p_view_state->m_factor;
	}
	// 滚轮缩放
	auto _mat_is_hover = false;
	for (auto _mat : p_materials_creating)
		if (_mat->isHover()) _mat_is_hover = true;
	if (p_view_state->m_is_hovered && !_mat_is_hover) {
		auto _wheel = GIO.MouseWheel;
		if (_wheel < -FLT_EPSILON)
			p_view_state->m_factor_target *= 1 - p_options->m_zoom_ratio;
		if (_wheel > FLT_EPSILON)
			p_view_state->m_factor_target *= 1 + p_options->m_zoom_ratio;
		ImVec2 mouseWPosPre = (GIO.MousePos - p_view_state->m_window_position) / p_view_state->m_factor;
		p_view_state->m_factor_target = ImClamp(p_view_state->m_factor_target, p_options->m_min_zoom, p_options->m_max_zoom);
		p_view_state->m_factor = ImLerp(p_view_state->m_factor, p_view_state->m_factor_target, p_options->m_zoom_lerp_factor);
		ImVec2 mouseWPosPost = (GIO.MousePos - p_view_state->m_window_position) / p_view_state->m_factor;
		// 指向鼠标位置进行缩放
		if (ImGui::IsMousePosValid()) {
			p_view_state->m_position += (mouseWPosPost - mouseWPosPre);
		}
	}
}

void Editor::MaterialEditor::handleEditor() {
	if (!p_scene) return;
	// 检查shader是否重建
	for (auto _shader : p_shaders_creating) {
		if (_shader->isReloadInMaterialEditor()) {
			_shader->isReloadInMaterialEditor() = false;
			handleAllFunc();
		}
	}

	if (ImGui::Button("+")) {
		ImGui::OpenPopup("Material Editor Add");
	} ImGui::SameLine();
	/*if (ImGui::Button("Compile All Shader")) {
		handleAllShaders();
	} ImGui::SameLine();*/
	/*if (ImGui::Button("Compile All Material")) {
		handleAllMaterials();
	} ImGui::SameLine();*/
	if (ImGui::Button("Preview")) {
		handleAllFunc();
	} ImGui::SameLine();
	if (ImGui::Button("Generate in Scene")) {
		handleAllGenerate();
	}
	if (ImGui::BeginPopup("Material Editor Add")) {
		if (false) {
			// Add Pipeline
			m_create_flags[CreateType::_Pipeline] = ImGui::Selectable("Pipeline");
			ThirdParty::imgui::help("可视化操作自定义渲染管线,输出为VkPipeline");
			// m_create_flags[CreateType::_RenderPass]  = ImGui::Selectable("RenderPass");
			// ThirdParty::imgui::help("可视化操作渲染流程,输出为VkRenderPass");
			m_create_flags[CreateType::_File] = ImGui::Selectable("File");
			ThirdParty::imgui::help("着色器文件容器, 输出为Shader");
		}
		else {
			m_create_flags[CreateType::_Shader] = ImGui::Selectable("Shader");
			ThirdParty::imgui::help("文件输入,输出待定");
			m_create_flags[CreateType::_Material] = ImGui::Selectable("Material");
			ThirdParty::imgui::help("材质, 输出为Material");
			m_create_flags[CreateType::_Texture] = ImGui::Selectable("Texture");
			ThirdParty::imgui::help("纹理, 输出为Texture");
			// Data------------------------------------
			{
				if (ImGui::BeginMenu("Data")) {
					if (ImGui::Selectable("Camera")) {
						m_create_flags[CreateType::_Data_Camera] = true;
					}
					ThirdParty::imgui::help("摄像机数据");
					//---------------------------
					if (ImGui::Selectable("Model")) {
						m_create_flags[CreateType::_Data_Model] = true;
					}
					ThirdParty::imgui::help("模型数据");
					//---------------------------
					if (ImGui::BeginMenu("Light")) {
						auto _lights = p_scene->getLights();
						for (auto _light : _lights) {
							if (ImGui::Selectable(_light.first.c_str())) {
								m_create_flags[CreateType::_Data_Light] = true;
								p_light_property_new = (RLightProperty*)_light.second->getSceneObjectProperty();
							}
						}
						ImGui::EndMenu();
					}
					ThirdParty::imgui::help("光照数据");
					//---------------------------
					if (ImGui::Selectable("Scene")) {
						m_create_flags[CreateType::_Data_Scene] = true;
					}
					ThirdParty::imgui::help("场景数据");
					ImGui::EndMenu();
				}
			}
			// 根据Shader生成的Struct------------------------------------
			{
				if (ImGui::BeginMenu("UniformBuffer")) {
					auto _nodes = Material::g_struct_nodes;
					for (auto _nodes_ite : _nodes) {
						auto _mat = _nodes_ite.first;
						auto _bindings = _nodes_ite.second;
						ImGui::SeparatorText(_mat->getSceneName().c_str());
						for (auto _binding : _bindings) {
							if (ImGui::Selectable(_binding.first.c_str())) {
								m_create_flags[CreateType::_Common_Struct] = true;
								p_common_struct_new = _nodes[_mat][_binding.first]();
							}
						}
					}
					ImGui::EndMenu();
				}
			}
			// 根据Shader生成的Struct------------------------------------
			{
				if (ImGui::BeginMenu("UniformBuffer No Input")) {
					auto _nodes = Material::g_datas_nodes;
					for (auto _nodes_ite : _nodes) {
						auto _mat = _nodes_ite.first;
						auto _bindings = _nodes_ite.second;
						ImGui::SeparatorText(_mat->getSceneName().c_str());
						for (auto _binding : _bindings) {
							if (ImGui::Selectable(_binding.first.c_str())) {
								m_create_flags[CreateType::_Data_Struct] = true;
								p_common_struct_new = _nodes[_mat][_binding.first]();
							}
						}
					}
					ImGui::EndMenu();
				}
			}

		}
		ImGui::EndPopup();
	}
	// 创建mat到场景
	if (ImGui::BeginPopup("Createing Material")) {
		static char name[256] = "new Material";
		ImGui::InputText("Name", name, 256); ImGui::SameLine();
		if (ImGui::Button("OK")) {
			Scene::addShader(name, p_material_in_scene->getShader());
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (checkCreateFlag(m_create_flags[CreateType::_Shader])) {
		std::string _name = "Shader";
		_name.append(std::to_string(p_id_generate->generateUUID()));
		auto _obj = new Shader(_name);
		createNode(_obj);
		p_shaders_creating.insert(_obj);
	}
	// 检查Create Material
	if (checkCreateFlag(m_create_flags[CreateType::_Material])) {
		auto _obj = new Core::Resource::Material();
		createNode(_obj);
		p_materials_creating.insert(_obj);
		auto _cam = new Camera(m_canvas_size, "Material Preview Camera");
		_cam->setMainCamera();
		_cam->getProperty()->m_rotate_R = 8.0f;
		_cam->getProperty()->m_move_speed = 0.01f;
		_cam->getProperty()->update();
		p_cameras_createting[_obj] = _cam;
	}
	if (checkCreateFlag(m_create_flags[CreateType::_Data_Camera])) {
		auto _obj = new Core::Resource::CameraProperty();
		createNode(_obj);
	}
	if (checkCreateFlag(m_create_flags[CreateType::_Data_Model])) {
		auto _obj = new Core::Resource::RMeshProperty();
		createNode(_obj);
	}
	if (checkCreateFlag(m_create_flags[CreateType::_Data_Light])) {
		if (p_light_property_new) {
			auto _obj = p_light_property_new;
			createNode(_obj);
			p_light_property_new = nullptr;
		}
	}
	if (checkCreateFlag(m_create_flags[CreateType::_Data_Scene])) {
		auto _obj = Scene::p_root_scene->getRootScene()->getLast();
		createNode(_obj);
	}
	if (checkCreateFlag(m_create_flags[CreateType::_Common_Struct]) || checkCreateFlag(m_create_flags[CreateType::_Data_Struct])) {
		if (p_common_struct_new) {
			p_common_structs_createing.insert(p_common_struct_new);
			createNode(p_common_struct_new);
			p_common_struct_new = nullptr;
		}
	}
	if (checkCreateFlag(m_create_flags[CreateType::_Texture])) {
		auto _obj = new TextureEXT();
		createNode(_obj);
		p_texture_ext_createing.insert(_obj);
	}
	
	// 弃用
	if (false) {
		if (checkCreateFlag(m_create_flags[CreateType::_Pipeline])) {
			auto _obj = new ThirdParty::Pipeline("Pipeline");
			createNode(_obj);
			m_create_flags[CreateType::_PipelineLayout] = true;
			m_create_flags[CreateType::_RenderPass] = true;
		}
		if (checkCreateFlag(m_create_flags[CreateType::_PipelineLayout])) {
			auto _obj = new ThirdParty::PipelineLayout("Pipeline Layout");
			createNode(_obj);
		}
		if (checkCreateFlag(m_create_flags[CreateType::_RenderPass])) {
			auto _obj = new Core::Resource::RRenderPass();
			createNode(_obj);
		}
		Debug(); // 2024年6月5日23:56:13 彻底改完，完善了IBL，增加了HDRI转CUBE Map
	}
}

void Editor::MaterialEditor::handleLoading() {
	if (!m_file_path_loading.empty()) {
		auto _path = std::filesystem::path(m_file_path_loading);
		auto _path_entry = std::filesystem::directory_entry(_path);
		auto _suffix = _path.extension().string();
		// Shader File
		if (SUFFIX_SHADER.contains(_suffix)) {
			auto* _shader_file = new Core::FileInfo(_path_entry);
			createNode(_shader_file);
		}
		// Texture
		if (SUFFIX_IMAGE.contains(_suffix)) {
			auto* _file = new Core::FileInfo(_path_entry);
			createNode(_file);
		}
	}
	m_file_path_loading = "";
}

void Editor::MaterialEditor::createNode(ZResource* _obj) {
	auto _temp = _obj->getTemplate();
	p_material_delegate->addTemplate(_temp);
	auto _node = ThirdParty::imgui::Node::createNode(_obj->getJsonTitle(), p_material_delegate->getTemplateIndex(_temp));
	_node->m_pos = -p_view_state->m_position + (p_view_state->m_canvas_size / p_view_state->m_factor / 2.0f);
	p_material_delegate->addNode(_node);
}

void Editor::MaterialEditor::createNode(Core::CommonStruct* _struct)
{
	_struct->initTemplate();
	auto _temp = _struct->p_template;
	p_material_delegate->addTemplate(_temp);
	auto _node = ThirdParty::imgui::Node::createNode(_struct->m_struct_name, p_material_delegate->getTemplateIndex(_temp));
	_node->m_pos = -p_view_state->m_position + (p_view_state->m_canvas_size / p_view_state->m_factor / 2.0f);
	p_material_delegate->addNode(_node);
}

void Editor::MaterialEditor::createNode(Core::Resource::RObjectProperty* _prop)
{
	_prop->initTemplate();
	auto _temp = _prop->p_template;
	p_material_delegate->addTemplate(_temp);
	auto _node = ThirdParty::imgui::Node::createNode(_prop->m_name, p_material_delegate->getTemplateIndex(_temp));
	_node->m_pos = -p_view_state->m_position + (p_view_state->m_canvas_size / p_view_state->m_factor / 2.0f);
	p_material_delegate->addNode(_node);
}

void Editor::MaterialEditor::createNode(Core::FileInfo* _obj) {
	auto _temp = _obj->getTemplate();
	p_material_delegate->addTemplate(_temp);
	auto _node = ThirdParty::imgui::Node::createNode("File", p_material_delegate->getTemplateIndex(_temp));
	_node->m_pos = -p_view_state->m_position + (p_view_state->m_canvas_size / p_view_state->m_factor / 2.0f);
	p_material_delegate->addNode(_node);
}

void Editor::MaterialEditor::createNode(ThirdParty::vkObject* _obj) {
	auto _temp = _obj->getTemplate();
	p_material_delegate->addTemplate(_temp);
	auto _node = ThirdParty::imgui::Node::createNode(_obj->getLabel(), p_material_delegate->getTemplateIndex(_temp));
	_node->m_pos = -p_view_state->m_position + (p_view_state->m_canvas_size / p_view_state->m_factor / 2.0f);
	p_material_delegate->addNode(_node);
}

bool Editor::MaterialEditor::checkCreateFlag(bool& _flag) {
	bool _rel = _flag;
	_flag = false;
	return _rel;
}

void Editor::MaterialEditor::handleLink()
{
	auto _links = p_material_delegate->p_links;
	for (auto _link : _links) {
		auto _node_input = p_material_delegate->getNode(_link->m_input_node_index);
		auto _node_output = p_material_delegate->getNode(_link->m_output_node_index);
		auto _temp_input = p_material_delegate->getTemplate(_node_input->m_template_index);
		auto _temp_output = p_material_delegate->getTemplate(_node_output->m_template_index);
		auto _slot_input = _temp_input->p_inputs[_link->m_input_slot_index];
		auto _slot_output = _temp_output->p_outputs[_link->m_output_slot_index];
		// 数据连接
		_slot_input->p_data = _slot_output->p_data;
		_slot_input->pp_data = _slot_output->pp_data;
		if (_slot_output->m_name_2)
			_slot_input->setName2(_slot_output->m_name_2);
		if (_slot_output->m_name_3)
			_slot_input->setName3(_slot_output->m_name_3);
	}
}

bool Editor::MaterialEditor::handleAllShaders()
{
	handleLink();
	bool _success = true;
	for (auto _obj : p_shaders_creating)
	{
		// 销毁绑定函数
		Scene::delDrawFunc(_obj);
		// 将使用该Shader的Material置为unReady()
		for (auto _mat : p_materials_creating)
			if (_mat->getShader() == _obj)
				_mat->isReady() = false;
		_obj->initObjFromTemplate();
		_success &= _obj->isReady();
		Scene::p_root_scene->getRootScene()->getLast()->reloadMaterial(_obj);
	}
	return _success;
}

void Editor::MaterialEditor::handleAllMaterials()
{
	handleLink();
	for (auto _obj : p_materials_creating)
	{
		Scene::delDrawFunc(_obj->getShader());
		// 如果当前帧被记录，则消除当前帧
		auto _cam = p_cameras_createting[_obj];
		auto _cmds = vkInfo.m_main_command_buffers;
		for (auto _cmd = _cmds.begin(); _cmd < _cmds.end(); _cmd++) {
			for (int i = 0; i < vkInfo.m_frame_count; i++) {
				if (*_cmd == _cam->getVkCommandBuffer(i)) {
					_cmds.erase(_cmd);
					break;
				}
			}
		}
		_obj->initObjFromTemplate();
	}
}

void Editor::MaterialEditor::handleAllTextures()
{
	handleLink();
	for (auto _tex : p_texture_ext_createing) {
		_tex->initObjFromTemplate();
	}
}

void Editor::MaterialEditor::handleAllFunc()
{
	handleAllTextures();
	if (!handleAllShaders()) return;
	handleAllMaterials();
	handleLink();
	auto _mats = p_materials_creating;
	// 处理Material Struct的连接
	for (auto _str : p_common_structs_createing) {
		_str->initObjFromTemplate();
	}
	// 创建bindData
	for (auto _mat : _mats) {
		_mat->initDrawFunc();
	}
}

void Editor::MaterialEditor::handleAllGenerate()
{
	for (auto _mat : p_materials_creating) {
		p_material_in_scene = _mat;
		ImGui::OpenPopup("Createing Material");
	}
}

Editor::MaterialDelegate::~MaterialDelegate()
{
	for (auto& node : p_nodes) ThirdParty::imgui::Node::deleteNode(node);
	p_nodes.clear();
	for (const auto& link : p_links) delete link;
	p_links.clear();
	// 数据Template放在obj中进行销毁
	// for (const auto& temp : p_templates) delete temp;
	// p_templates.clear();
}

bool Editor::MaterialDelegate::allowedLink(NodeIndex from, NodeIndex to)
{
	return true;
}

void Editor::MaterialDelegate::delNode(Node* _node) {
	uint32_t index = 0;
	for (const auto& _n : p_nodes) {
		if (_n == _node) {
			p_nodes.erase(p_nodes.begin() + index);
			break;
		}
		index++;
	}
}

void Editor::MaterialDelegate::addNode(Node* _node) {
	p_nodes.push_back(_node);
}

void Editor::MaterialDelegate::selectNode(NodeIndex nodeIndex, bool selected)
{
	p_nodes[nodeIndex]->m_select = selected;
}

void Editor::MaterialDelegate::moveSelectedNodes(const ImVec2 delta)
{
	for (auto& node : p_nodes)
	{
		if (!node->m_select)
		{
			continue;
		}
		node->m_pos += delta;
	}
}

void Editor::MaterialDelegate::addLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex)
{
	Link* p_link = new Link;
	p_link->m_input_node_index = inputNodeIndex;
	p_link->m_input_slot_index = inputSlotIndex;
	p_link->m_output_node_index = outputNodeIndex;
	p_link->m_output_slot_index = outputSlotIndex;
	p_links.push_back(p_link);
}

void Editor::MaterialDelegate::delLink(LinkIndex linkIndex)
{
	p_links.erase(p_links.begin() + linkIndex);
}

uint32_t Editor::MaterialDelegate::getTemplateIndex(Template* _temp) {
	return p_templates[_temp];
}

void Editor::MaterialDelegate::addTemplate(Template* _temp) {
	p_templates[_temp] = p_templates.size();
	p_templates_indexed[p_templates[_temp]] = _temp;
}
// 删除模板
void Editor::MaterialDelegate::delTemplate(Template* _temp) {
	auto _ite = p_templates.find(_temp);
	auto _ite2 = p_templates_indexed.find(_ite->second);
	p_templates.erase(_ite);
	p_templates_indexed.erase(_ite2);
}
void Editor::MaterialDelegate::delTemplate(TemplateIndex _index) {
	auto _ite = p_templates_indexed.find(_index);
	auto _ite2 = p_templates.find(_ite->second);
	p_templates_indexed.erase(_ite);
	p_templates.erase(_ite2);
}

const size_t Editor::MaterialDelegate::getTemplateCount()
{
	return p_templates.size();
}

ThirdParty::imgui::Template* Editor::MaterialDelegate::getTemplate(TemplateIndex index)
{
	return p_templates_indexed[index];
}

const size_t Editor::MaterialDelegate::getNodeCount()
{
	return p_nodes.size();
}

ThirdParty::imgui::Node* Editor::MaterialDelegate::getNode(NodeIndex index)
{
	return p_nodes[index];
}

const size_t Editor::MaterialDelegate::getLinkCount()
{
	return p_links.size();
}

ThirdParty::imgui::Link* Editor::MaterialDelegate::getLink(LinkIndex index)
{
	return p_links[index];
}

static bool _first = true;
static bool _second = false;
static bool _third = false;
static bool _forth = false;
static bool _fifth = false;
void Editor::MaterialEditor::Debug() {
	// 明天就答辩了，今天找到工作了，2024年6月5日17:28:26，永别了牢笼
	// 又回来了。。2024年5月29日21:29:45
	// return;	// 永别了 牢笼 2024年5月28日22:35:42
	if (_fifth) {
		_fifth = !_fifth;
		p_material_delegate->addLink(7, 0, 4, 0);	// cam Pos -> ubo 7
		p_material_delegate->addLink(7, 1, 8, 0);	// lightPos -> ubo 7
		p_material_delegate->addLink(7, 2, 8, 1);	// lightColor -> ubo 7
		p_material_delegate->addLink(7, 3, 5, 2);	// baseColor -> ubo 7
		p_material_delegate->addLink(7, 4, 8, 2);	// lightRadiance -> ubo 7
		p_material_delegate->addLink(7, 5, 8, 3);	// lightStrength -> ubo 7
		p_material_delegate->addLink(3, 2, 7, 0);	// ubo 7 -> Material
		
		p_material_delegate->addLink(3, 11, 9, 0);	// ubo 9 -> Material
		p_material_delegate->addLink(3, 12, 10, 0);	// ubo 10 -> Material

	}
	if (_forth) {
		_forth = !_forth;
		// DirectionLight
		auto _lights = p_scene->getLights();
		for (auto _light : _lights) {
			p_light_property_new = (RLightProperty*)_light.second->getSceneObjectProperty();
			if (p_light_property_new) {
				auto _obj = p_light_property_new;
				createNode(_obj);
				p_light_property_new = nullptr;
			}
		}	// 8

		for (auto _mat : p_materials_creating) {
			auto _str2 = _mat->getSlots()[2]->getSlotDatas();
			for (auto _str : _str2) { // 9 10
				p_common_struct_new = Material::g_datas_nodes[_mat][_str.first]();
				if (p_common_struct_new) {
					p_common_structs_createing.insert(p_common_struct_new);
					createNode(p_common_struct_new);
					p_common_struct_new = nullptr;
				}
			}
		}
		_fifth = true;
	}
	if (_third) {
		_third = !_third;
		p_material_delegate->addLink(6, 0, 5, 1);	// model -> ubo
		p_material_delegate->addLink(6, 1, 4, 1);	// v  -> ubo
		p_material_delegate->addLink(6, 2, 4, 2);	// p  -> ubo
		p_material_delegate->addLink(6, 3, 4, 3);	// vp -> ubo
		p_material_delegate->addLink(3, 1, 6, 0);	// ubo -> mat
		_forth = true;
	}
	if (_second) {
		_second = !_second;
		// 连线
		p_material_delegate->addLink(2, 0, 0, 0);	// vert -> shader
		p_material_delegate->addLink(2, 2, 1, 0);	// frag -> shader
		p_material_delegate->addLink(3, 0, 2, 0);	// shader -> mat
		if (!handleAllShaders()) return;
		handleAllMaterials();
		// 创建UBO
		for (auto _mat : p_materials_creating) {
			auto _str = _mat->getSlots()[0]->getSlotDatas();
			for (auto str : _str) {
				p_common_struct_new = Material::g_struct_nodes[_mat][str.first]();
				if (p_common_struct_new) {// 6 、 7
					p_common_structs_createing.insert(p_common_struct_new);
					createNode(p_common_struct_new);
					p_common_struct_new = nullptr;
				}
			}
			break;
		}
		//m_create_flags[CreateType::_Common_Struct] = true;		
		_third = true;
	}

	if (_first) {
		_first = !_first;
		m_file_path_loading = ThirdParty::Core::getModuleCurrentDirectory() + "\\resources\\glsl\\pbr\\pbr.vs";
		handleLoading();	// 0
		m_file_path_loading = ThirdParty::Core::getModuleCurrentDirectory() + "\\resources\\glsl\\pbr\\pbr.fs";
		handleLoading();	// 1

		m_create_flags[CreateType::_Shader] = true;			// 2
		m_create_flags[CreateType::_Material] = true;		// 3
		m_create_flags[CreateType::_Data_Camera] = true;	// 4
		m_create_flags[CreateType::_Data_Model] = true;		// 5
		
		_second = true;
	}
}