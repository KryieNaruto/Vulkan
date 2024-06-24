#include "sub/Material.h"
#include "Scene.h"
#include <ThirdParty/imgui/imgui_impl_vulkan.h>
#include <Core/include/core_util_function.h>

Core::Resource::Material::Material(Shader* _shader, const std::string& _name /*= "Material"*/)
	:RMaterial(), SceneObject(_name)
{
	setShader(_shader);
	m_scene_type = MATERIAL;
	setZResourceProperty(p_property);
	setShaderMaterial(p_shader, this);
}

Core::Resource::Material::Material(RMaterial* _rmat)
	:RMaterial(), SceneObject(_rmat->getName())
{
	setShader(_rmat->getShader())->bindTextures(_rmat->getTextures());
	m_scene_type = MATERIAL;
	//memcpy(p_template, _rmat->p_template, sizeof(*p_template));
	setZResourceProperty(p_property);
	setShaderMaterial(p_shader, this);
}

Core::Resource::Material::Material()
	:RMaterial(), SceneObject("Material Editor Creating")
{
	initTemplate();
}

Core::Resource::Material::~Material()
{
}

void Core::Resource::Material::propertyEditor()
{
	// �������editor_global_func::RenderFunc��
}

Core::Resource::Material* Core::Resource::Material::addEmptyTexture(uint32_t _set_index, const std::string& _binding_name)
{
	auto _binding = getBiniding(_set_index, _binding_name);
	auto _current_index = _binding->getTextureIndex();
	auto _tex = ThirdParty::TextureEXT::getTextureEXT();
	_binding->bindData(_tex, _current_index);
	_binding->isEmpty(true);		// ����BindingΪ������
	m_texture_count += 1;			// Mat tex����+1
	return this;
}

ThirdParty::SlotData* Core::Resource::Material::getBiniding(uint32_t _set_index, uint32_t _binding_index)
{
	auto _set = getSlots()[_set_index];
	auto _binding = _set->getSlotData(_binding_index);
	return _binding;
}

ThirdParty::SlotData* Core::Resource::Material::getBiniding(uint32_t _set_index, const std::string& _binding_name)
{
	auto _set = getSlots()[_set_index];
	auto _binding = _set->getSlotData(_binding_name);
	return _binding;
}

void Core::Resource::Material::setShaderMaterial(Shader* _shader, Material* _mat)
{
	g_shader_materials[_shader].insert(_mat);
}

void Core::Resource::Material::reloadMaterial(Shader* _shader)
{
	for (const auto& [shader, _mats] : g_shader_materials) {
		if (shader == _shader) {
			for (const auto& _mat : _mats)
				_mat->reload();
			break;
		}
	}
}

void Core::Resource::Material::initTemplate()
{
	// input
	auto _input_0 = new ThirdParty::imgui::SlotTemplateInput();
	_input_0->setName("Shader");
	_input_0->p_data = nullptr;
	// ��ʾ����Ԥ��
	auto _func = [this, _input_0]()->void {
		if (ImGui::CollapsingHeader("PreView")) {
			if (m_preview.empty()) return;
			m_hover = ImGui::IsWindowHovered();
			ImGui::Image(m_preview[m_vkInfo.m_current_frame], { _input_0->m_final_size.x,_input_0->m_final_size.x });
		}
		};
	_input_0->m_functional = _func;
	p_template->addInputSlotTemplate({ _input_0 });
	p_template->m_original_input_count = p_template->p_inputs.size();

	// ouput
	auto _output_0 = new ThirdParty::imgui::SlotTemplateOutput();
	_output_0->setName("Final Out");
	_output_0->p_data = this;
	p_template->addOutputSlotTemplates({ _output_0 });
	p_template->m_original_output_count = p_template->p_outputs.size();
}

void Core::Resource::Material::initObjFromTemplate()
{
	if (p_template->p_inputs[0]->p_data != nullptr) {
		auto _shader = (Shader*)p_template->p_inputs[0]->p_data;
		if (_shader->isReady())
			setShader(_shader);
		else return;
	}
	// ��ʾUBO input
	// �²��, �;ɲ�۱Ƚϣ������¼������ӵ㡣������ȫ���Ͽ�������
	std::vector<ThirdParty::imgui::SlotTemplateInput*> _new_inputs;
	std::vector<ThirdParty::imgui::SlotTemplateInput*> _new_inputs_final;	// ���ն������inputs
	{
		for (const auto& _slot_ite : p_slots) {
			auto _set_id = _slot_ite.first;
			auto _slot = _slot_ite.second;
			for (const auto& _slot_data_ite : _slot->getBindingNames()) {
				auto _slot_data = _slot->getSlotData(_slot_data_ite.first);
				auto _input_name = _slot_data->getBindingName();
				auto _input_i = new ThirdParty::imgui::SlotTemplateInput();
				_input_i->setName(_input_name);
				_input_i->p_data = nullptr;
				// ��ʾUBO������ͼ
				if (_slot_data->isUniformBuffer()) {
					// ����UBO�����ԣ������������ݵ�Struct�ڵ�
					g_struct_nodes[this][_input_name] = [_slot_data]()->Core::CommonStruct* {
						auto _struct_name = _slot_data->getMemberStructOrdered().first;
						auto _struct_members = _slot_data->getMemberStructOrdered().second;
						auto _com = new CommonStruct(_struct_name, _struct_members);		// ��δ���٣��ᵼ���ڴ�й©��
						return _com;
						};
					g_datas_nodes[this][_input_name] = [_slot_data]()->Core::DataStruct* {
						return new DataStruct(_slot_data);		// ��δ���٣��ᵼ���ڴ�й©��
						};

					auto _func = [_slot_data]()->void {
						auto _struct_name = _slot_data->getMemberStructOrdered().first;
						auto _struct_members = _slot_data->getMemberStructOrdered().second;
						if (ImGui::CollapsingHeader(_struct_name.c_str())) {
							for (auto _struct_member : _struct_members) {
								auto _member_name = _struct_member.first;		// ��Ա����
								auto _member_detail = _struct_member.second;	// ��Աsize / offset
								ImGui::Text((_member_name + "[size:%d][offset:%d]").c_str(), _member_detail.first, _member_detail.second);
							}
						}
						};
					_input_i->m_functional = _func;
				}
				else if (_slot_data->isTexture()) {
					auto _func = [_slot_data, _input_i]()->void {
						auto _struct_name = _slot_data->getBindingName();
						if (ImGui::CollapsingHeader(_struct_name.c_str())) {
							if (TextureEXT::_final_sets.contains(_input_i->p_data)) {
								auto _tex = (TextureEXT*)_input_i->p_data;
								auto _width = _input_i->m_final_size.x;
								auto _height = _width / _tex->getAspect();
								ImGui::Image(TextureEXT::_final_sets[_tex], { _width,_height });
							}
						}
						};
					_input_i->m_functional = _func;
				}
				_new_inputs.push_back(_input_i);
			}
		}
	}
	_new_inputs_final.insert(_new_inputs_final.end(), _new_inputs.begin(), _new_inputs.end());
	// ��ʼ�Ƚ��¾ɽڵ�
	// ������Shader�󣬽�UBO���ݿ��ӻ�����Ϊ_input��
	// ����ԭʼinput, �������input������Shader������������Ϊ����input
	auto _input_size = p_template->p_inputs.size();
	if (_input_size > p_template->m_original_input_count) {
		for (int i = p_template->m_original_input_count; i < _input_size; i++) {
			if (i < 0) continue;
			auto _inputs = p_template->p_inputs;
			// �ɽڵ� ���½ڵ����˳���ж�, �����ͬ����link���ֲ���
			auto _old_input = p_template->p_inputs[i];
			// ����½ڵ㲻�㣬��ǰ�ڵ��Լ�֮��Ŀ���ֱ�����٣� ͬʱ��������
			if (_new_inputs.size() - 1 < i - p_template->m_original_input_count) {
				// ��������
				if (ThirdParty::imgui::Delegate::g_delegate) {
					auto _delegate = ThirdParty::imgui::Delegate::g_delegate;
					for (int _node_index = 0; _node_index < _delegate->getNodeCount(); _node_index++) {
						auto _node = _delegate->getNode(_node_index);
						auto _template = _delegate->getTemplate(_node->m_template_index);
						// ���ҵ�ǰ�ڵ��Template
						if (_template == p_template) {
							for (int _link_index = 0; _link_index < _delegate->getLinkCount(); _link_index++) {
								auto _links = _delegate->getLink(_link_index);
								// ���ٵ�ǰ�ڵ������
								if (_links->m_input_node_index == _node_index || _links->m_output_node_index == _node_index) {
									if (_links->m_input_slot_index == i) _delegate->delLink(_link_index);
									if (_links->m_output_slot_index == i) _delegate->delLink(_link_index);
								}
							}
						}
					}
				}
				// ɾ��input
				delete _old_input;
				p_template->p_inputs[i] = nullptr;
				// ����
				p_template->p_inputs.erase(_inputs.begin() + i);
				i--;
				continue;
			}

			auto _new_input = _new_inputs[i - p_template->m_original_input_count];
			if (strcmp(_new_input->m_name, _old_input->m_name) == 0) {
				// ��ͬ�����滻��
				// ɾ��input
				delete _old_input;
				// �滻
				p_template->p_inputs[i] = _new_input;
				// ������˳����룬����ÿ��ֻ��Ҫ����ͷ�����ɡ�
				_new_inputs_final.erase(_new_inputs_final.begin());
				continue;
			}
			// ��ͬ�� �򽫵�ǰtemplate�滻Ϊ�µģ�������ǰ��input�ڵ����ơ�
			else {
				_inputs.insert(_inputs.begin() + i, _new_input);
				_new_inputs_final.erase(_new_inputs_final.begin());
				// ͬʱ�޸�linkΪ�ɽڵ��index + 1
				if (ThirdParty::imgui::Delegate::g_delegate) {
					auto _delegate = ThirdParty::imgui::Delegate::g_delegate;
					for (int _node_index = 0; _node_index < _delegate->getNodeCount(); _node_index++) {
						auto _node = _delegate->getNode(_node_index);
						auto _template = _delegate->getTemplate(_node->m_template_index);
						// ���ҵ�ǰ�ڵ��Template
						if (_template == p_template) {
							for (int _link_index = 0; _link_index < _delegate->getLinkCount(); _link_index++) {
								auto _links = _delegate->getLink(_link_index);
								// ��ǰ�ڵ�
								if (_links->m_input_node_index == _node_index || _links->m_output_node_index == _node_index) {
									if (_links->m_input_slot_index == i) _links->m_input_slot_index++;		// ����+1
									if (_links->m_output_slot_index == i) _links->m_output_slot_index++;	// ����+1
								}
							}
						}
					}
				}
			}
			delete p_template->p_inputs[i];
			p_template->p_inputs[i] = nullptr;
		}
	}

	// ����½ڵ�
	p_template->addInputSlotTemplates(_new_inputs_final);
}

void Core::Resource::Material::initDrawFunc()
{
	if (p_shader != nullptr) {
		Scene::delDrawFunc(p_shader);
		std::vector<std::function<Shader_Data_Binding_Func_Type>> t_functionals;
		// ����draw func
		{
			int _slot_index = 1;
			for (const auto& _slot_ite : p_slots) {
				auto _set_id = _slot_ite.first;
				auto _bind_id = 0;
				auto _slot = _slot_ite.second;
				for (const auto& _slot_data_ite : _slot->getSlotDatas()) {
					auto _input_i = p_template->p_inputs[_slot_index++];	// ��ȡ���
					if (_input_i->p_data == nullptr && _input_i->pp_data == nullptr) {
						_bind_id++;
						continue;
					}

					auto _slot_data = _slot_data_ite.second;
					// UBO
					if (_slot_data->isUniformBuffer()) {
						auto _func = SHADER_DATA_BINDING_FUNC_LAMBDA_WITH_ARGS(_set_id, _bind_id, _input_i, this) {
							auto _input_data = strcmp(_input_i->m_name_3, "CommonStruct") == 0 ? (Core::CommonStruct*)_input_i->p_data : (Core::DataStruct*)_input_i->p_data;
							auto mat = (Material*)_mat;
							auto cam = (Camera*)_camera;
							auto model = (Model*)_scene_obj;
							auto mesh = (Mesh*)_mesh;
							void* _data = nullptr;

							auto _member = _input_data->m_members;
							for (auto _tuple : _member) {
								if (mat) {
									auto _name_2 = _input_data->m_names[std::get<0>(_tuple)].first;
									auto _name_3 = _input_data->m_names[std::get<0>(_tuple)].second;
									// ������
									if (cam) {
										if (m_hover) {
											cam->focus();
											cam->onUpdate();
										}
										else cam->unfocus();
										auto property = cam->getProperty();
										property->update();
										if (_name_2 == "View")
											_data = &property->m_mat4_view;
										else if (_name_2 == "Projection")
											_data = &property->m_mat4_proj;
										else if (_name_2 == "View-Projection")
											_data = &property->m_mat4_view_proj;
										else if (_name_2 == "Pos" && _name_3 == "Camera")
											_data = &property->m_vec3_pos;
									}
									// ��ģ������
									if (((model && mesh == nullptr) || (mesh)) && _data == nullptr) {
										Mesh* mesh_in_model = nullptr;
										if (model && mesh == nullptr)
											mesh_in_model = model->getMesh(_mesh_index);
										else if (mesh)
											mesh_in_model = mesh;
										auto property = mesh_in_model->getProperty();
										property->update();

										if (_name_2 == "Pos" && _name_3 == "Model")
											_data = &property->m_vec3_pos;
										else if (_name_2 == "Model")
											_data = &property->m_model;
										else if (_name_2 == "BaseColor")
											_data = &property->m_vec3_color;
									}
									if (_data) {
										mat->bindData(_set_id, _bind_id, _data);
										_data = nullptr;
									}
									// �����ǰ�����������_data == nullptr�����Ƿǳ�������
									else if (!_data) {
										auto _ptr = std::get<3>(_tuple);
										if (_ptr) {
											// ����֮���Կ�����_set_id,_bind_id����Ϊ input����˳��������
											mat->bindData(_set_id, _bind_id, _ptr);
										}
									}
								}
							}
						};
						t_functionals.push_back(_func);
					}
					else {
						auto _func = SHADER_DATA_BINDING_FUNC_LAMBDA_WITH_ARGS(_set_id, _bind_id, _input_i) {
							auto mat = (Material*)_mat;
							if (mat) {
								// ����
								auto _tex = (TextureEXT*)_input_i->p_data;
								if (_input_i->pp_data != nullptr)
									_tex = *((TextureEXT**)_input_i->pp_data);
								mat->bindTexture(_set_id, _bind_id, _tex);
							}
						};
						t_functionals.push_back(_func);
					}
					_bind_id++;
				}
				_set_id++;
			}
		};
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA_WITH_ARGS(t_functionals) {
			for (auto _t : t_functionals)
				_t(_scene, _mat, _scene_obj, _mesh_index, _mesh, _camera);
		};
		Scene::addDrawFunc(p_shader, _func);
	}
	m_is_ready = true;
}
