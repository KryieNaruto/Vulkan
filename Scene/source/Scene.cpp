#include "Scene.h"
using namespace Core::Resource;

#define SHADER_SKYBOX "shader_skybox"
#define SHADER_DEPTH "shader_depth"
#define SHADER_OUTPUT_DEPTH "shader_output"
#define SHADER_DRAW_LINE "shader_draw_line"
#define SHADER_CONVOLUTION "convolution"
#define SHADER_BRDF "brdf"
#define SHADER_PREFILTER "prefilter"
#define SHADER_HDRI_TO_CUBE "hdri_to_cube"

Scene* Core::Resource::Scene::p_root_scene;              // 根场景
std::unordered_map<std::string, Shader*> Core::Resource::Scene::p_shaders;		 // 所有经过预加载的shader
std::unordered_map<Shader*, Material*> Core::Resource::Scene::p_materials;   // 所有经过预加载的shader
std::unordered_map<Shader*, Shader_Data_Binding_Func>  Core::Resource::Scene::p_draw_func;
std::unordered_map<TextureEXT*, TextureEXT*>  Core::Resource::Scene::p_convolution_textures;
std::unordered_map<TextureEXT*, TextureEXT*> Core::Resource::Scene::p_prefilter_textures;  // 预滤波环境图，key=环境贴图,value = LOD滤波环境贴图
TextureEXT* Core::Resource::Scene::p_brdf_texture = nullptr; // brdf积分图
std::vector<std::string>  Core::Resource::Scene::m_hdri_texture_paths;
std::unordered_map<std::string, TextureEXT*>  Core::Resource::Scene::p_hdri_textures;
std::unordered_map<TextureEXT*, TextureEXT*> Core::Resource::Scene::p_cube_textures;   // 转化后的立方体贴图
Core::Resource::Scene::~Scene()
{
	for (const auto& [_name, _model] : p_models) {
		Model::removeModel(_model);
		delete _model;
	}
	for (const auto& [_name, _model] : p_lights) {
		delete _model;
	}
	// 销毁所有model
	for (auto i = Model::p_models.begin(); i != Model::p_models.end(); i++) {
		delete* i;
	}
	Model::p_models.clear();
}

Core::Resource::SubCamera* Scene::addDepthSubCamera(const VkExtent3D& _canvas_size, const std::string& _name /*= "SubCamera"*/)
{
	auto _sub = addSubCamera(_canvas_size, "Sub Camera");
	_sub->setCanvasSize(_canvas_size)
		->setRRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_2));
	for (uint32_t i = 0; i < m_vkInfo.m_frame_count; i++) {
		// att 0
		auto _depth = new TextureEXT(
			_canvas_size,
			ThirdParty::findDepthFormat(),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_SAMPLE_COUNT_1_BIT,
			"Depth Attachment"
		);
		// att 1
		auto _img = new TextureEXT(
			_canvas_size,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_SAMPLE_COUNT_1_BIT,
			"Color Attachment"
		);

		_sub->addAttachment(DEPTH_ATTACHMENT, _depth)
			->addAttachment(COLOR_ATTACHMENT, _img);
	}
	_sub->createFramebuffer()
		->createCommandBuffer()
		->endInitialize();
	return _sub;
}

Core::Resource::SubCamera* Scene::addSubCamera(const VkExtent3D& _canvas_size, const std::string& _name /*= "SubCamera"*/)
{
	SubCamera* _sub_camera = new SubCamera(_name);
	_sub_camera->setCanvasSize(_canvas_size);
	_sub_camera->setScene(this);
	p_camera[_sub_camera->getSceneName()] = _sub_camera;
	p_world_scenes[_sub_camera->getSceneName()] = _sub_camera;
	return _sub_camera;
}

void Scene::preloadRenderPass()
{
	const auto& vkInfo = m_vkInfo;
	// 预加载RenderPass
	// 输出Depth ：VK_SAMPLE_COUNT_1_BIT
	{
		RRenderPass* p_render_normal = new RRenderPass();
		p_render_normal
			->addAttachment({
					0,
					ThirdParty::findDepthFormat(),
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				})
			->addAttachment({
					1,
					VK_FORMAT_R8G8B8A8_UNORM,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				})
			//---------------------------- subpass 0
			->addNewSubpass()
			->setSubpassProperty_addDpethStencilAttachment(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0)
			//---------------------------- subpass 1
			->addNewSubpass()
			->setSubpassProperty_addInputAttachment(0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1)
			->setSubpassProperty_addColorAttachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1)
			// dependency external - 0
			->addDependency(
				VK_SUBPASS_EXTERNAL, 0,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,

				VK_ACCESS_MEMORY_READ_BIT,
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT)
			// dependency 0 - 1
			->addDependency(
				0, 1,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,

				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT)
			->addDependency(
				0, VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,

				VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT)
			->create();
		Core::Resource::RRenderPass::addRRenderPass(RENDER_PASS_2, p_render_normal);
	}
}

Core::Resource::Scene* Scene::addSpotLight(const glm::vec3& _pos, const glm::vec4& _light_color)
{
	Light* _light = new Light(LIGHT_TYPE::SPOT, _light_color, "Spot Light");
	_light
		->setShape(new Sphere())
		->setShader(getShader(SHADER_LIGHT));
	_light->setScene(this);
	_light->setPos(_pos);
	addLight(_light);
	return this;
}

Core::Resource::Scene* Scene::getLast()
{
	if (this == p_root_scene)
		return p_last;
}

Core::Resource::Shape* Scene::addShape(SCENE_TYPE _shape_type, const glm::vec3& _pos)
{
	Shape* _shape = nullptr;
	switch (_shape_type)
	{
	case Core::Resource::MODEL:
		break;
	case Core::Resource::SHAPE:
		break;
	case Core::Resource::PLANE:
		_shape = new Plane();
		break;
	case Core::Resource::CUBE:
		_shape = new Cube();
		break;
	case Core::Resource::SPHERE:
		_shape = new Sphere();
		break;
	case Core::Resource::MESH:
		break;
	case Core::Resource::CAMERA:
		break;
	case Core::Resource::SCENE:
		break;
	case Core::Resource::SHADER:
		break;
	case Core::Resource::MATERIAL:
		break;
	default:
		break;
	}
	if (_shape) {
		_shape->setShader(getShader(SHADER_DEFAULT));
		_shape->setScene(this);
		_shape->getProperty()->m_vec3_pos = _pos;
		addModel(_shape);
	}
	return _shape;
}

void Core::Resource::Scene::initTemplate()
{
	// output 辐照度环境贴图
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput;
		_output->setName("IrradiancesMap");
		_output->setName2("IrradiancesMap");
		_output->setName3("TextureEXT");
		_output->p_data = nullptr;
		_output->pp_data = malloc(sizeof(TextureEXT*));
		*((TextureEXT**)_output->pp_data) = getSkyboxHDRITextureCube_Convolution();
		p_template->addOutputSlotTemplate(_output);
	}
	// 环境滤波
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput;
		_output->setName("EnvirmentPrefilterMap");
		_output->setName2("EnvirmentPrefilterMap");
		_output->setName3("TextureEXT");
		_output->p_data = getSkyboxHDRITextureCube_Prefilter();
		_output->pp_data = malloc(sizeof(TextureEXT*));
		*((TextureEXT**)_output->pp_data) = getSkyboxHDRITextureCube_Prefilter();
		p_template->addOutputSlotTemplate(_output);
	}
	// brdf积分图
	{
		auto _output = new ThirdParty::imgui::SlotTemplateOutput;
		_output->setName("Brdf Lut");
		_output->setName2("Brdf Lut");
		_output->setName3("TextureEXT");
		_output->p_data = p_brdf_camera->getCurrentTextureEXT(COLOR_ATTACHMENT);
		_output->pp_data = malloc(sizeof(TextureEXT*));
		*((TextureEXT**)_output->pp_data) = p_brdf_camera->getCurrentTextureEXT(COLOR_ATTACHMENT);
		p_template->addOutputSlotTemplate(_output);
	}
}

TextureEXT* Scene::getSkyboxHDRITextureCube_Convolution()
{
	return p_convolution_textures[getSkyboxHDRITextureCube()];
}

TextureEXT* Scene::getSkyboxHDRITextureCube_Prefilter()
{
	return p_prefilter_textures[getSkyboxHDRITextureCube()];
}

TextureEXT* Scene::getSkyboxHDRITextureCube()
{
	return p_cube_textures[getSkyboxHDRITexture()];
}

Core::Resource::Scene* Scene::addSkybox(const std::string& _path, TextureEXT* _tex)
{
	m_hdri_texture_paths.push_back(_path);
	p_hdri_textures[_path] = _tex;
	return this;
}

TextureEXT* Scene::getSkyboxHDRITexture()
{
	if (m_skybox_shape == CUBE && m_skybox_shape_last == CUBE)
		return TextureEXT::getTextureEXT("default-skybox");
	else {
		if (m_hdri_texture_paths.empty()) return nullptr;
		return TextureEXT::getTextureEXT(m_hdri_texture_paths[m_skybox_hdri_index]);
	}
}

Core::Resource::Scene* Scene::convolutionSkybox()
{
	static glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, 1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f,  1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f,  1.0f,  0.0f)),
	};
	static glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	static bool first = true;
	if (first) {
		projectionMatrix[1][1] *= -1;
	}
	// 如果是hdri,则将其转化为cubemap
	auto _cube = p_hdri_to_cube_camera->getCurrentTextureEXT(COLOR_ATTACHMENT);
	if (m_skybox_shape == SPHERE) {
		if (p_hdri_to_cube_camera->isReady()) {
			for (int _count = 0; _count < 6; _count++) {
				// 设置绘制视角
				p_hdri_to_cube_camera->setTempFrameBuffer(p_hdri_to_cube_camera->getFrameBuffer(_count));
				//p_convolution_camera->setPosAndTarget({ 0,0,0 }, _targets[_count]);
				p_hdri_to_cube_camera->setTempViewMatrix(captureViews[_count]);
				p_hdri_to_cube_camera->setTempProjectionMatrix(projectionMatrix);
				auto _cmd = p_hdri_to_cube_camera->onRenderStartImmediately();
				auto _mat = p_materials[getShader(SHADER_HDRI_TO_CUBE)];
				if (_mat) {
					auto _draw_func = getDrawFunc(_mat->getShader());
					Core::Resource::draw_shape(SPHERE, this, _mat, _cmd, _draw_func, p_hdri_to_cube_camera);
				}
				p_hdri_to_cube_camera->onRenderEndImmediately(_cmd);
			}
			// 保存cubemap
			VkExtent3D _size = { 512,512,1 };
			auto _tex = new TextureEXT(
				_size,
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				6,
				VK_SAMPLE_COUNT_1_BIT,
				"HDRI To CubeTexture COPY"
			);
			_tex->copyAllTextureEXT(_cube);
			p_cube_textures[getSkyboxHDRITexture()] = _tex;
		}
	}

	// 卷积环境贴图
	if (p_convolution_camera->isReady()) {
		for (int _count = 0; _count < 6; _count++) {
			// 设置绘制视角
			p_convolution_camera->setTempFrameBuffer(p_convolution_camera->getFrameBuffer(_count));
			//p_convolution_camera->setPosAndTarget({ 0,0,0 }, _targets[_count]);
			p_convolution_camera->setTempViewMatrix(captureViews[_count]);
			p_convolution_camera->setTempProjectionMatrix(projectionMatrix);
			if (!p_convolution_camera->isReady()) return this;
			auto _cmd = p_convolution_camera->onRenderStartImmediately();
			auto _mat = p_materials[getShader(SHADER_CONVOLUTION)];
			if (_mat) {
				auto _draw_func = getDrawFunc(_mat->getShader());
				Core::Resource::draw_shape(CUBE, this, _mat, _cmd, _draw_func, p_convolution_camera);
			}
			p_convolution_camera->onRenderEndImmediately(_cmd);
		}
		// 保存卷积结果
		VkExtent3D _size = { 32,32,1 };
		auto _tex = new TextureEXT(
			_size,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6,
			VK_SAMPLE_COUNT_1_BIT,
			"Convolution CubeTexture COPY"
		);
		_tex->copyAllTextureEXT(p_convolution_camera->getCurrentTextureEXT(COLOR_ATTACHMENT));
		p_convolution_textures[getSkyboxHDRITextureCube()] = _tex;
	}
	// 预滤波环境贴图
	if (p_prefilter_camera) {
		for (int _lod = 0; _lod < roughnessLog; _lod++) {
			unsigned int mipWidth = 128 * std::pow(0.5, _lod);
			unsigned int mipHeight = 128 * std::pow(0.5, _lod);
			float _roughness = (float)_lod / (float)(roughnessLog - 1);
			for (int _count = 0; _count < 6; _count++) {
				auto _index = _lod * 6 + _count;
				p_prefilter_camera->setTempFrameBuffer(p_prefilter_camera->getFrameBuffer(_index));
				p_prefilter_camera->setTempViewMatrix(captureViews[_count]);
				p_prefilter_camera->setTempProjectionMatrix(projectionMatrix);
				p_prefilter_camera->setTempRenderArea({ {0,0},{mipWidth,mipHeight} });
				auto _cmd = p_prefilter_camera->onRenderStartImmediately();
				auto _mat = p_materials[getShader(SHADER_PREFILTER)];
				_mat->bindData(0, 0, &_roughness);
				if (_mat) {
					auto _draw_func = getDrawFunc(_mat->getShader());
					Core::Resource::draw_shape(CUBE, this, _mat, _cmd, _draw_func, p_prefilter_camera);
				}
				p_prefilter_camera->onRenderEndImmediately(_cmd);
			}
		}
		// 保存卷积结果
		VkExtent3D _size = { 128,128,1 };
		auto _tex = new TextureEXT(
			_size,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6,
			VK_SAMPLE_COUNT_1_BIT,
			"Prefilter CubeTexture COPY",
			roughnessLog
		);
		_tex->copyAllTextureEXT(p_prefilter_camera->getCurrentTextureEXT(COLOR_ATTACHMENT));
		// 保存滤波结果
		p_prefilter_textures[getSkyboxHDRITextureCube()] = _tex;
	}
	first = false;
	return this;
}

Core::Resource::SubCamera* Scene::addCubeSubCamera(const VkExtent3D& _canvas_size, const std::string& _name /*= "SubCamera"*/)
{
	auto _sub = addSubCamera(_canvas_size, _name);
	_sub->setCanvasSize(_canvas_size)
		->setRRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_NODEPTH));
	_sub->getHidden() = true;
	return _sub;
}

void Scene::delDrawFunc(Shader* _shader)
{
	auto _ite = p_draw_func.find(_shader);
	if (_ite != p_draw_func.end()) {
		p_draw_func.erase(_ite);
	}
}

Core::Resource::Scene* Scene::addPointLight(const glm::vec3& _pos, const glm::vec4& _light_color)
{
	Light* _light = new Light(LIGHT_TYPE::POINT, _light_color, "Point Light");
	_light
		->setShape(new Sphere())
		->setShader(getShader(SHADER_LIGHT));
	_light->setColor(_light_color, _light_color.w);
	_light->setScene(this);
	_light->setPos(_pos);
	addLight(_light);
	return this;
}

Core::Resource::Scene* Scene::addAmbientLight(const glm::vec3& _light_color, float _strength)
{
	Light* _light = new Light(LIGHT_TYPE::AMBIENT, { _light_color,_strength }, "Ambient Light");
	_light
		->setShape(new Sphere())
		->setShader(getShader(SHADER_LIGHT));
	_light->setColor(_light_color, _strength);
	_light->setScene(this);
	addLight(_light);
	return this;
}

void Scene::onUpdate()
{
	switch (m_skybox_shape_index)
	{
	case 0: m_skybox_shape = CUBE; break;
	case 1:m_skybox_shape = SPHERE; break;
	default:
		break;
	}
}

const std::unordered_map<Shader*, Material*>& Scene::getMaterials()
{
	return p_materials;
}

const std::unordered_map<std::string, Shader*>& Scene::getShaders()
{
	return p_shaders;
}

Core::Resource::Scene* Scene::reloadMaterial(Shader* _shader)
{
	std::set<void*> _mat_reloaded;
	// 模型
	for (const auto& [_shader_mat, _mats] : p_model_materials) {
		if (_shader == _shader_mat) {
			for (const auto& _mat : _mats) {
				if (!_mat_reloaded.contains(_mat)) {
					_mat->reload();
					_mat_reloaded.insert(_mat);
				}
			}
		}
	}
	// All models
	{
		auto _models = p_root_scene->getRootScene()->getLast()->p_models;
		for (auto _ite : _models) {
			auto _mats_ite_ite = _ite.second->getMaterialsCopyCameraUse();
			for (auto _mats_ite : _mats_ite_ite) {
				for (auto _mats : _mats_ite.second) {
					if (_mats.first == _shader) {
						for (auto _mat : _mats.second) {
							if (!_mat_reloaded.contains(_mat)) {
								_mat->reload();
								_mat_reloaded.insert(_mat);
							}
						}
					}
				}
			}
		}
	}

	// 光源
	for (const auto& [_name, _light] : p_lights) {
		if (_light->getShader() == _shader) {
			for (const auto& _mat : _light->getShape()->getMaterials()) {
				if (!_mat_reloaded.contains(_mat)) {
					_mat->reload();
					_mat_reloaded.insert(_mat);
				}
			}
		}
	}
	// 材质
	for (const auto& [p_shader, _mat] : p_materials) {
		if (p_shader == _shader) {
			if (!_mat_reloaded.contains(_mat)) {
				_mat->reload();
				_mat_reloaded.insert(_mat);
			}
		}
	}

	return this;
}

Core::Resource::Scene* Scene::addLight(Light* _light)
{
	_light->setScene(this);
	p_lights[((Core::Resource::SceneObject*)_light)->getSceneName()] = _light;
	p_world_scenes[((Core::Resource::SceneObject*)_light)->getSceneName()] = _light;
	// 绑定Light与Camera
	auto _cam = addDepthSubCamera(m_canvas_size);
	_cam->setPosAndTarget(_light);
	_cam->setPorjectType(_light->getLightType());
	_light->setRenderCamera(_cam);
	return this;
}

Core::Resource::Scene* Scene::addModel(Model* _model)
{
	p_models[_model->getSceneName()] = _model;
	p_world_scenes[_model->getSceneName()] = _model;
	// 保存Material
	auto _mats = _model->getMaterials();
	p_model_materials[_model->getShader()].insert(p_model_materials[_model->getShader()].end(), _mats.begin(), _mats.end());
	return this;
}

Core::Resource::Scene* Scene::loadModelFromFile(const std::string& _path)
{
	Model* p_model = new Model(_path);
	p_model->setShader(getShader(SHADER_DEFAULT));
	p_model->setScene(this);
	addModel(p_model);
	return this;
}

Core::Resource::Scene* Scene::removeLight(const std::string& _name)
{
	auto ite = p_lights.find(_name);
	if (ite != p_lights.end()) {
		delete ite->second;
		p_lights.erase(ite);
		p_world_scenes.erase(p_world_scenes.find(_name));
	}
	return this;
}

Core::Resource::Scene* Scene::addDirectionLight(const glm::vec3& _direction, const glm::vec4& _light_color)
{
	Light* _light = new Light(LIGHT_TYPE::PARALLEL, _light_color, "Direction Light");
	_light
		->setShape(new Sphere())
		->setShader(getShader(SHADER_LIGHT));
	_light->setDirection(_direction, LIGHT_TYPE::PARALLEL);
	_light->setPos(_direction);
	_light->setColor(_light_color, _light_color.w);
	_light->setScene(this);
	addLight(_light);
	return this;
}

void Scene::propertyEditor()
{
	static std::unordered_map<TextureEXT*, VkDescriptorSet> _sets;
	auto _hdri_tex = getSkyboxHDRITexture();
	if (!_sets.contains(_hdri_tex))
		_sets[getSkyboxHDRITexture()] = ImGui_ImplVulkan_AddTexture(
			ThirdParty::Sampler::g_all_samplers[SAMPLER_REPEAT]->getVkSampler(),
			_hdri_tex->getVkImageView(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	auto _size = ImGui::GetWindowSize();
	ImVec2 _image_size = _size;
	_image_size.y = _image_size.x / _hdri_tex->getAspect();
	ImGui::Image(_sets[_hdri_tex], _image_size);
}

void Scene::addShader(const std::string& _name, Shader* _shader)
{
	// 如果存在该Shader,则更新所有的Material
	if (p_shaders.contains(_name)) {
		p_root_scene->getRootScene()->getLast()->reloadMaterial(_shader);
	}
	else {
		p_shaders[_name] = _shader;
	}
}

void Scene::addDrawFunc(Shader* _shader, Shader_Data_Binding_Func _draw_func)
{
	p_draw_func[_shader] = _draw_func;
}

Shader_Data_Binding_Func Scene::getDrawFunc(Shader* _shader)
{
	auto _ite = p_draw_func.find(_shader);
	if (_ite != p_draw_func.end()) {
		return _ite->second;
	}

	else if (_shader == nullptr) return nullptr;
	else {
		LOG_WARN("Failed to find bind func:{0}", _shader->getName());
		return nullptr;
	}
}

void Scene::onRender()
{
	if (m_is_skip) {
		m_is_skip = false;
		return;
	}
	// 更新光源位置
	for (const auto& [_name, _light] : p_lights)
		_light->update();

	// 渲染深度
	{
		uint32_t _cam_index = 0;
		for (const auto& [_name, _sub] : p_camera) {
			if (_sub->getHidden()) continue;
			_sub->onUpdate();
			// 如果是主摄像
			if (_sub == p_main_camera) continue;

			auto _subCamera = (SubCamera*)_sub;
			auto _plane_title = std::string(SHAPE_PLANE) + "Camera" + std::to_string(_cam_index++);
			// 只有点光源/平行光/Main Depth Camera/触发深度渲染
			auto _light_scene = _subCamera->getSceneObjectTarget();
			if (_light_scene) {
				auto _type = _light_scene->getSceneObejctType();
				if (_type == SCENE_TYPE::LIGHT) {
					auto _light = (Light*)_light_scene;
					if (_light->getLightType() == LIGHT_TYPE::AMBIENT) continue;
					// 当光源位置变换才进行深度图更新。
					//if (!_light->isPosChange()) continue;
					if (_subCamera->isReady()) {
						_subCamera
							->setClearValues({ _subCamera->getProperty()->m_canvas_dp,  _subCamera->getProperty()->m_canvas_bg })
							->onRenderStart();
						auto _cmd = _subCamera->getVkCommandBuffer();
						// subpass 0 渲染深度
						auto shader_depth = getShader(SHADER_DEPTH);
						if (shader_depth) {
							auto _m_draw_func = getDrawFunc(shader_depth);
							if (_m_draw_func != NULL) {
								// 渲染模型
								for (const auto& [name, _model] : p_models) {
									if (!_model->isReady() || _model->getHidden()) continue;
									_model
										->setRenderCamera(_subCamera)
										->setShader(shader_depth, true)
										->draw(_cmd, _m_draw_func);
								}
							}
						}
						// subpass 1 深度转化为RGBA 供调试使用
						vkCmdNextSubpass(_cmd, VK_SUBPASS_CONTENTS_INLINE);
						// 将深度图R -> RGBA8
						auto shader_output_depth = getShader(SHADER_OUTPUT_DEPTH);
						if (shader_output_depth) {
							auto _m_draw_func = getDrawFunc(shader_output_depth);
							auto _plane = getPlane(_plane_title);
							if (_m_draw_func && _plane) {
								_plane
									->setRenderCamera(_subCamera)
									->setShader(shader_output_depth, true)
									->draw(_cmd, _m_draw_func);
							}
						}
						_subCamera->onRenderEnd();
					}
				}
				else if (_type == SCENE_TYPE::CAMERA) {
					auto _light = (Camera*)_light_scene;
					if (_subCamera->isReady()) {
						_subCamera
							->setClearValues({ _subCamera->getProperty()->m_canvas_dp,  _subCamera->getProperty()->m_canvas_bg })
							->onRenderStart();
						auto _cmd = _subCamera->getVkCommandBuffer();
						// subpass 0 渲染深度
						auto shader_depth = getShader(SHADER_DEPTH);
						if (shader_depth) {
							auto _m_draw_func = getDrawFunc(shader_depth);
							if (_m_draw_func != NULL) {
								// 渲染模型
								for (const auto& [name, _model] : p_models) {
									if (!_model->isReady()) continue;
									_model
										->setRenderCamera(_subCamera)
										->setShader(shader_depth, true)
										->draw(_cmd, _m_draw_func);
								}
							}
						}
						// subpass 1 深度转化为RGBA 供调试使用
						vkCmdNextSubpass(_cmd, VK_SUBPASS_CONTENTS_INLINE);
						// 将深度图R -> RGBA8
						auto shader_output_depth = getShader(SHADER_OUTPUT_DEPTH);
						if (shader_output_depth) {
							auto _m_draw_func = getDrawFunc(shader_output_depth);
							auto _plane = getPlane(_plane_title);
							if (_m_draw_func && _plane) {
								_plane
									->setRenderCamera(_subCamera)
									->setShader(shader_output_depth, true)
									->draw(_cmd, _m_draw_func);
							}
						}
						_subCamera->onRenderEnd();
					}
				}
			}
		}
	}
	if (p_main_camera) {
		uint32_t _cam_index = 0;
		if (!p_main_camera->isReady()) return;
		p_main_camera->onRenderStart();
		auto _cmd = p_main_camera->getVkCommandBuffer();
		// 绘制光源
		if (m_show_light) {
			for (const auto& [_name, _light] : p_lights) {
				if (_light->getHidden()) continue;
				auto _func = getDrawFunc(_light->getShader());
				_light->draw(_cmd, _func);
			}
		}

		// 绘制模型
		for (const auto& [_name, _model] : p_models) {
			if (_model->getHidden()) continue;
			auto _func = getDrawFunc(_model->getShader());
			_model->setRenderCamera(p_main_camera);
			_model->draw(_cmd, _func);
		}

		// 天空盒
		if (m_show_skybox) {
			auto m_skybox = p_materials[getShader(SHADER_SKYBOX)];
			if (m_skybox) {
				// 更新ubo数据
				auto _draw_func = getDrawFunc(m_skybox->getShader());
				m_skybox->bindTexture(0, 1, getSkyboxHDRITexture());
				if (m_use_convolution) {
					m_skybox->bindTexture(0, 2, getSkyboxHDRITextureCube_Convolution());
					if (getSkyboxHDRITextureCube_Convolution() != nullptr)
						m_skybox_shape = CUBE;
					//m_skybox->bindTexture(0, 2, p_prefilter_textures[TextureEXT::getTextureEXT("default-skybox")]);
				}
				else {
					m_skybox_shape = SPHERE;
					m_skybox->bindTexture(0, 2, getSkyboxHDRITextureCube());
				}
				// TODO: 绘制立方体
				Core::Resource::draw_shape(m_skybox_shape, this, m_skybox, _cmd, _draw_func, p_main_camera);
			}
		}

		// 网格
		if (m_show_2d_mesh) {
			auto m_0 = p_materials[getShader(SHADER_2D_MESH)];
			if (m_0) {
				// 更新ubo数据
				auto _m_draw_func = getDrawFunc(m_0->getShader());
				if (_m_draw_func != NULL)
					_m_draw_func(this, m_0, nullptr, 0, nullptr, nullptr);
				m_0->bind(_cmd);
				vkCmdDraw(_cmd, 6, 1, 0, 0);
			}
		}

		// DEBUG
		// 绘制视椎体
		if (m_is_debug) {
			for (const auto& [_name, _sub] : p_camera) {
				if (_sub->getHidden()) continue;
				_sub->onUpdate();
				// 如果是主摄像
				if (_sub == p_main_camera) continue;
				auto _plane_title = std::string(SHAPE_PLANE) + "Camera_DEBUG" + std::to_string(_cam_index++);
				auto _plane = getPlane(_plane_title);
				_plane->setScene(this);
				auto _shader = getShader(SHADER_DRAW_LINE);
				auto _m_draw_func = getDrawFunc(_shader);
				if (_m_draw_func && _plane) {
					_plane
						->setRenderCamera(_sub)
						->setShader(_shader, true)
						->draw(_cmd, _m_draw_func);
				}
			}
		}

		p_main_camera->onRenderEnd();
	}
	// 卷积辐照度
	if (m_use_convolution) {
		if (!p_convolution_textures.contains(getSkyboxHDRITextureCube()) || p_convolution_textures[getSkyboxHDRITextureCube()] == nullptr) {
			convolutionSkybox();
		}
		*((TextureEXT**)p_template->p_outputs[0]->pp_data) = getSkyboxHDRITextureCube_Convolution();
		*((TextureEXT**)p_template->p_outputs[1]->pp_data) = getSkyboxHDRITextureCube_Prefilter();
		*((TextureEXT**)p_template->p_outputs[2]->pp_data) = p_brdf_camera->getCurrentTextureEXT(COLOR_ATTACHMENT);
	}
	// 卷积
	if (false) {
		static std::unordered_map<VkImageView, VkDescriptorSet> _sets;
		static int _count = 0;
		static int _count_max = 2;
		static bool _d = true;
		if (_count++ > _count_max && _d) {
			convolutionSkybox();
			_d = false;
		}
		if (p_convolution_camera->isReady() && false) {
			if (ImGui::Begin("Debug")) {
				if (ImGui::Button("Convolution"))
					convolutionSkybox();
				if (p_convolution_camera) {
					auto _texs = p_hdri_to_cube_camera->getCurrentTextureEXT(TEXTURE_TYPE::COLOR_ATTACHMENT)->getVkImageViews();
					for (auto _tex : _texs) {
						if (!_sets.contains(_tex))
							_sets[_tex] =
							ImGui_ImplVulkan_AddTexture(ThirdParty::Sampler::g_all_samplers[SAMPLER]->getVkSampler(), _tex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
						ImGui::Image(_sets[_tex], { 100,100 });
					}
				}
			}
			ImGui::End();
		}
		if (p_brdf_camera->isReady() && true) {
			if (ImGui::Begin("Debug BRDF")) {
				auto _texs = p_brdf_camera->getCurrentTextureEXT(TEXTURE_TYPE::COLOR_ATTACHMENT)->getVkImageViews();
				for (auto _tex : _texs) {
					if (!_sets.contains(_tex))
						_sets[_tex] =
						ImGui_ImplVulkan_AddTexture(ThirdParty::Sampler::g_all_samplers[SAMPLER]->getVkSampler(), _tex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					ImGui::Image(_sets[_tex], { 100,100 });
				}
			}ImGui::End();
		}
	}
}

Core::Resource::Shader* Scene::getShader(const std::string& _name)
{
	auto _ite = p_shaders.find(_name);
	if (_ite != p_shaders.end())
		return _ite->second;
	else {
		LOG_ERROR("Failed to find shader:{0}", _name);
		return nullptr;
	}
}

void Scene::prebindData()
{
	Material* shader_2d_mesh = new Material(getShader(SHADER_2D_MESH));
	p_materials[getShader(SHADER_2D_MESH)] = shader_2d_mesh;
	Material* shader_skybox = new Material(getShader(SHADER_SKYBOX));
	p_materials[getShader(SHADER_SKYBOX)] = shader_skybox;
	// 预设球形天空盒HDR
	Material* shader_depth = new Material(getShader(SHADER_DEPTH));
	p_materials[getShader(SHADER_DEPTH)] = shader_depth;
	Material* shader_draw_line = new Material(getShader(SHADER_DRAW_LINE));
	p_materials[getShader(SHADER_DRAW_LINE)] = shader_draw_line;
	// 卷积
	Material* shader_convolution = new Material(getShader(SHADER_CONVOLUTION));
	p_materials[getShader(SHADER_CONVOLUTION)] = shader_convolution;
	Core::Resource::getCube()->setShader(shader_convolution->getShader());
	Core::Resource::getSphere()->setShader(shader_convolution->getShader());
	// brdf
	Material* shader_brdf = new Material(getShader(SHADER_BRDF));
	p_materials[getShader(SHADER_BRDF)] = shader_brdf;
	Core::Resource::getPlane()->setShader(shader_brdf->getShader());
	// prefilter 预滤波
	Material* shader_prefilter = new Material(getShader(SHADER_PREFILTER));
	p_materials[getShader(SHADER_PREFILTER)] = shader_prefilter;
	Core::Resource::getCube()->setShader(shader_prefilter->getShader());
	// hdri -> cubemap
	Material* shader_hdri_to_cube = new Material(getShader(SHADER_HDRI_TO_CUBE));
	p_materials[getShader(SHADER_HDRI_TO_CUBE)] = shader_hdri_to_cube;
	Core::Resource::getSphere()->setShader(shader_hdri_to_cube->getShader());
	// 数据绑定
	// SHADER_2D_MESH
	{
		auto p_shader = getShader(SHADER_2D_MESH);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
			auto scene = (Core::Resource::Scene*)_scene;
			Core::Resource::Camera* _cam = scene->getMainCamera();

			// 绑定数据 ubo- Common
			if (_mat && _cam) {
				const auto& _cam_data = _cam->getProperty();
				auto mat = ((Core::Resource::Material*)_mat);
				mat->bindData(0, 0, &_cam_data->m_mat4_view_proj_inverse)
					->bindData(0, 0, &_cam_data->m_mat4_view_proj)
					->bindData(0, 0, &_cam_data->m_vec3_pos)
					->bindData(0, 0, &_cam_data->m_zNear)
					->bindData(0, 0, &_cam_data->m_zFar);
			} };

		addDrawFunc(p_shader, _func);
	}

	// SHADER_DEFAULT
	{
		auto p_shader = getShader(SHADER_DEFAULT);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
			auto scene = (Core::Resource::Scene*)_scene;
			Core::Resource::Camera* _cam = scene->getMainCamera();

			// 绑定数据 ubo- Common
			if (_mat && _cam) {
				Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
				Core::Resource::Mesh* mesh =
					_mesh != nullptr ? (Core::Resource::Mesh*)_mesh : ((Core::Resource::Model*)_scene_obj)->getMesh(_mesh_index);
				const auto& _cam_data = _cam->getProperty();
				auto _mesh_data = mesh->getProperty();
				// LIGHT
				//-----------------------------------------------------
				size_t _light_index = 0;
				if (((Core::Resource::Scene*)((Core::Resource::Model*)_scene_obj)->getScene())->getShow_light()) {
					for (const auto& _light : scene->getLights()) {
						Core::Resource::Light* light = ((Core::Resource::Light*)_light.second);
						Core::Resource::Camera* _sub_cam = (Core::Resource::Camera*)light->getRenderCamera();
						const auto& _light_data = light->getProperty();
						if (!light->getHidden()) {
							//-----------------------------------------------------
							mat->bindData(0, 2, &_light_data->m_vec3_pos, _light_index)
								->bindData(0, 2, &_light_data->m_front, _light_index)
								->bindData(0, 2, &_light_data->m_color, _light_index)
								->bindData(0, 2, &_light_data->m_strength, _light_index)
								->bindData(0, 2, &_light_data->m_type, _light_index)
								->bindData(0, 2, &_light_data->m_attenuation, _light_index)
								->bindData(0, 2, &_light_data->m_radians, _light_index)
								->bindData(0, 2, &_light_data->m_cut_off, _light_index)
								->bindData(0, 2, &_light_data->m_cut_off_outer, _light_index)
								->bindData(0, 2, &_sub_cam->getProperty()->m_mat4_view_proj, _light_index);
							mat->bindTexture(2, 0, _sub_cam->getCurrentTextureEXT(DEPTH_ATTACHMENT), _light_index);
							_light_index++;
						}
					}
				}
				// COMMON
				//-----------------------------------------------------
				mat->bindData(0, 0, &_cam_data->m_mat4_view)
					->bindData(0, 0, &_cam_data->m_mat4_proj)
					->bindData(0, 0, &_light_index)
					//-----------------------------------------------------
					->bindData(0, 1, &_cam_data->m_vec3_pos)
					->bindData(0, 1, &_cam_data->m_zNear)
					->bindData(0, 1, &_cam_data->m_zFar);
				// MODEL
				//-----------------------------------------------------
				auto _tex_count = std::min(_mesh_data->m_texture_count, (int)mat->getTextureCount());
				mat->bindData(1, 0, &_mesh_data->m_model)
					->bindData(1, 0, &_mesh_data->m_model_inverse)
					->bindData(1, 0, &_mesh_data->m_vec3_color)
					->bindData(1, 0, &_tex_count)
					->bindData(1, 0, &_mesh_data->m_shinness)
					->bindData(1, 0, &_mesh_data->m_has_shadow);
			}
		};
		addDrawFunc(p_shader, _func);
	}

	// LIGHT_DEFAULT
	{
		auto p_shader = getShader(SHADER_LIGHT);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
			auto scene = (Core::Resource::Scene*)_scene;
			Core::Resource::Camera* _cam = scene->getMainCamera();

			// 绑定数据 ubo->Common
			if (_mat && _cam) {
				Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
				Core::Resource::Light* light = ((Core::Resource::Light*)_scene_obj);
				auto mesh = light->getShape();
				if (mesh == nullptr) return;
				light->update();
				const auto& _cam_data = _cam->getProperty();
				const auto& _light_data = light->getProperty();
				const auto& _mesh_data = mesh->getProperty();
				mat->bindData(0, 0, &_cam_data->m_mat4_view)
					->bindData(0, 0, &_cam_data->m_mat4_proj)
					->bindData(0, 2, &_light_data->m_color)
					->bindData(1, 0, &_mesh_data->m_model);
			}
		};
		addDrawFunc(p_shader, _func);
	}

	// SHADER_SKYBOX
	{
		auto p_shader = getShader(SHADER_SKYBOX);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
			auto scene = (Core::Resource::Scene*)_scene;
			Core::Resource::Camera* _cam = scene->getMainCamera();
			const auto& _cam_data = _cam->getProperty();
			const auto& _sampler_repeat = Material::getSampler(SAMPLER_REPEAT)->getVkSampler();
			// 绑定数据 ubo->Common
			if (_mat && _cam) {
				Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
				mat->bindData(0, 0, &_cam_data->m_mat4_view)
					->bindData(0, 0, &_cam_data->m_mat4_proj)
					->bindData(0, 0, &scene->getSkyboxType())
					->bindData(0,1, _sampler_repeat);
			}
		};
		addDrawFunc(p_shader, _func);
	}

	// SHADER_DEPTH
	{
		auto p_shader = getShader(SHADER_DEPTH);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
			auto scene = (Core::Resource::Scene*)_scene;
			const auto& _model = ((Core::Resource::Model*)_scene_obj);
			if (_model->isReady()) {
				const auto& _cam = scene->getMainCamera();
				const auto& mesh = _mesh != nullptr ? (Core::Resource::Mesh*)_mesh : _model->getMesh(_mesh_index);
				const auto& _cam_data = (_camera != nullptr) ? ((Core::Resource::Camera*)_camera)->getProperty() : _cam->getProperty();
				const auto& _mesh_data = mesh->getProperty();
				// 绑定数据 ubo->Common
				if (_mat && _cam) {
					Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
					mat->bindData(0, 0, &_cam_data->m_mat4_view)
						->bindData(0, 0, &_cam_data->m_mat4_ortho)
						->bindData(0, 0, &_cam_data->m_mat4_view_ortho)
						->bindData(1, 0, &_mesh_data->m_model);
				}
			}
		};
		addDrawFunc(p_shader, _func);
	}

	// SHADER_OUTPUT_DEPTH
	{
		auto p_shader = getShader(SHADER_OUTPUT_DEPTH);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
		const auto & _cam = (Camera*)_camera;
		// 绑定数据 ubo->Common
		if (_mat && _cam) {
			auto _cam_data = _cam->getProperty();
			Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
			mat->bindTexture(0, 0, _cam->getCurrentTextureEXT(DEPTH_ATTACHMENT))
				->bindData(1, 0, &_cam_data->m_zNear)
				->bindData(1, 0, &_cam_data->m_zFar)
				->bindData(1, 0, &_cam_data->m_project_type);
		}
		};
		addDrawFunc(p_shader, _func);
	}
	// SHADER_CONVOLUTION
	{
		auto p_shader = getShader(SHADER_CONVOLUTION);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
		const auto & _cam = (Camera*)_camera;
		const auto& scene = (Scene*)_scene;
		// 绑定数据 ubo->Common
		if (_mat && _cam) {
			auto _cam_data = _cam->getProperty();
			Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
			auto _cubetex = scene->getSkyboxHDRITextureCube();
			mat
				->bindData(0, 0, &_cam_data->m_mat4_view)
				->bindData(0, 0, &_cam_data->m_mat4_proj)
				->bindTexture(0, 1, _cubetex);
		}
		};
		addDrawFunc(p_shader, _func);
	}
	// brdf
	{
		auto p_shader = getShader(SHADER_BRDF);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{};
		addDrawFunc(p_shader, _func);
	}
	// 预滤波环境贴图
	{
		auto p_shader = getShader(SHADER_PREFILTER);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
			const auto& _cam = (Camera*)_camera;
		const auto& scene = (Scene*)_scene;
		// 绑定数据 ubo->Common
		if (_mat && _cam) {
			auto _cam_data = _cam->getProperty();
			Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
			auto _cube_tex = scene->getSkyboxHDRITextureCube();
			mat
				->bindData(0, 0, &_cam_data->m_mat4_view)
				->bindData(0, 0, &_cam_data->m_mat4_proj)
				->bindTexture(0, 1, _cube_tex);
		}
		};
		addDrawFunc(p_shader, _func);
	}
	// hdri->cubemap
	{
		auto p_shader = getShader(SHADER_HDRI_TO_CUBE);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
			const auto& _cam = (Camera*)_camera;
		const auto& scene = (Scene*)_scene;
		auto _tex_hdri = scene->getSkyboxHDRITexture();
		// 绑定数据 ubo->Common
		if (_mat && _cam) {
			auto _cam_data = _cam->getProperty();
			Core::Resource::Material* mat = ((Core::Resource::Material*)_mat);
			mat
				->bindData(0, 0, &_cam_data->m_mat4_view)
				->bindData(0, 0, &_cam_data->m_mat4_proj)
				->bindTexture(0, 1, _tex_hdri);
		}
		};
		addDrawFunc(p_shader, _func);
	}
	// DEBUG
	// SHADER_DRAW_LINE
	{
		auto p_shader = getShader(SHADER_DRAW_LINE);
		Shader_Data_Binding_Func _func = SHADER_DATA_BINDING_FUNC_LAMBDA{
		const auto & _cam = (Camera*)_camera;
		const auto& _main_cam = ((Scene*)_scene)->getMainCamera();
			if (_mat && _cam) {
				auto _cam_data = _cam->getProperty();
				auto _main_cam_data = _main_cam->getProperty();
				auto mat = (Core::Resource::Material*)_mat;
				mat->bindData(0, 0, &_main_cam_data->m_mat4_view)
					->bindData(0, 0, &_main_cam_data->m_mat4_proj)
					->bindData(0, 0, &_cam_data->m_mat4_view_proj_inverse)
					->bindData(0, 0, &_cam_data->m_zNear)
					->bindData(0, 0, &_cam_data->m_zFar)
					->bindData(0, 0, &_cam_data->m_project_width)
					->bindData(0, 0, &_cam_data->m_project_height)
					->bindData(0, 0, &_cam_data->m_aspect)
					->bindData(0, 0, &_cam_data->m_vec3_pos)
					->bindData(0, 0, &_cam_data->m_project_type);
			}
		};
		addDrawFunc(p_shader, _func);
	}
}

void Scene::preloadShader()
{
	// 3D 网格
	const auto& app_res = ThirdParty::Core::getModuleCurrentDirectory() + "\\resources\\";

	addShader(SHADER_2D_MESH, new Shader(
		SHADER_2D_MESH,
		{
			{ThirdParty::ShaderType::vertex,app_res + "glsl\\shader_2d_mesh.vert"},
			{ThirdParty::ShaderType::fragment,app_res + "glsl\\shader_2d_mesh.frag"},
		}, false, true));

	// default 默认光照模型
	addShader(SHADER_DEFAULT, new Shader(
		SHADER_DEFAULT,
		{
			{ThirdParty::ShaderType::vertex,app_res + "glsl\\shader_default.vert"},
			{ThirdParty::ShaderType::fragment,app_res + "glsl\\shader_default.frag"},
		}));

	// 光源Shader
	addShader(SHADER_LIGHT, new Shader(
		SHADER_LIGHT,
		{
			{ThirdParty::ShaderType::vertex,app_res + "glsl\\shader_light.vert"},
			{ThirdParty::ShaderType::fragment,app_res + "glsl\\shader_light.frag"},
		}));

	// 天空盒Shader
	addShader(SHADER_SKYBOX, new Shader(
		SHADER_SKYBOX,
		{
			{ThirdParty::ShaderType::vertex,app_res + "glsl\\shader_skybox.vert"},
			{ThirdParty::ShaderType::fragment,app_res + "glsl\\shader_skybox.frag"},
		}));

	// Camera 视椎体Debug
	{
		auto _shader_0 = new Shader(SHADER_DRAW_LINE);
		{
			_shader_0
				->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
				->setDepthTest(false)
				->setDepthWrite(false)
				->createShader(
					{
						{ThirdParty::ShaderType::vertex,app_res + "glsl\\" + SHADER_DRAW_LINE + ".vert"},
						{ThirdParty::ShaderType::geometry,app_res + "glsl\\" + SHADER_DRAW_LINE + ".geom"},
						{ThirdParty::ShaderType::fragment,app_res + "glsl\\" + SHADER_DRAW_LINE + ".frag"},
					}
			);
		}
		addShader(SHADER_DRAW_LINE, _shader_0);
	}
	// Cube 立方体贴图卷积输出到立方体贴图
	{
		auto _shader_0 = new Shader(SHADER_CONVOLUTION);
		{
			_shader_0
				->setRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_NODEPTH)->getVkRenderPass())
				->setMultiSampler(false)
				->createShader(
					{
						{ThirdParty::ShaderType::vertex,app_res + "glsl\\pbr\\" + SHADER_CONVOLUTION + ".vs"},
						{ThirdParty::ShaderType::fragment,app_res + "glsl\\pbr\\" + SHADER_CONVOLUTION + ".fs"},
					}
			);
		}
		addShader(SHADER_CONVOLUTION, _shader_0);
	}
	// brdf
	{
		auto _shader_0 = new Shader(SHADER_BRDF);
		{
			_shader_0
				->setRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_RG_NODEPTH)->getVkRenderPass())
				->setMultiSampler(false)
				->createShader(
					{
						{ThirdParty::ShaderType::vertex,app_res + "glsl\\pbr\\" + SHADER_BRDF + ".vs"},
						{ThirdParty::ShaderType::fragment,app_res + "glsl\\pbr\\" + SHADER_BRDF + ".fs"},
					}
			);
		}
		addShader(SHADER_BRDF, _shader_0);
	}
	// 预滤波环境贴图
	{
		auto _shader_0 = new Shader(SHADER_PREFILTER);
		{
			_shader_0
				->setRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_NODEPTH)->getVkRenderPass())
				->setMultiSampler(false)
				->createShader(
					{
						{ThirdParty::ShaderType::vertex,app_res + "glsl\\pbr\\" + SHADER_PREFILTER + ".vs"},
						{ThirdParty::ShaderType::fragment,app_res + "glsl\\pbr\\" + SHADER_PREFILTER + ".fs"},
					}
			);
		}
		addShader(SHADER_PREFILTER, _shader_0);
	}
	// HDRI_TO_CUBE
	{
		auto _shader_0 = new Shader(SHADER_HDRI_TO_CUBE);
		{
			_shader_0
				->setRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_NODEPTH)->getVkRenderPass())
				->setMultiSampler(false)
				->createShader(
					{
						{ThirdParty::ShaderType::vertex,app_res + "glsl\\pbr\\" + SHADER_HDRI_TO_CUBE + ".vs"},
						{ThirdParty::ShaderType::fragment,app_res + "glsl\\pbr\\" + SHADER_HDRI_TO_CUBE + ".fs"},
					}
			);
		}
		addShader(SHADER_HDRI_TO_CUBE, _shader_0);
	}

	// RenderPass 输出深度
	{
		auto _render_pass = Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_2);
		// 深度图Shader
		auto _shader_0 = new Shader(SHADER_DEPTH);
		{
			_shader_0
				->setRenderPass(_render_pass->getVkRenderPass())
				->setDepthBias(VK_TRUE)
				->addDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS)
				->setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
				->setOpaque(true)
				->setCullMode(VK_CULL_MODE_FRONT_BIT)
				->setMultiSampler(false)
				->setSubpassIndex(0)
				->createShader(
					{
						{ThirdParty::ShaderType::vertex,app_res + "glsl\\shader_depth.vert"},
						{ThirdParty::ShaderType::fragment,app_res + "glsl\\shader_depth.frag"},
					});
		}
		addShader(SHADER_DEPTH, _shader_0);
		// 输出深度图
		auto _shader_1 = new Shader(SHADER_OUTPUT_DEPTH);
		{
			_shader_1
				->setRenderPass(_render_pass->getVkRenderPass())
				->setDepthBias(VK_TRUE)
				->addDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS)
				->setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
				->setOpaque(true)
				//->setCullMode(VK_CULL_MODE_FRONT_BIT)
				->setMultiSampler(false)
				->setSubpassIndex(1)
				->createShader(
					{
						{ThirdParty::ShaderType::vertex,app_res + "glsl\\shader_output.vert"},
						{ThirdParty::ShaderType::fragment,app_res + "glsl\\shader_output.frag"},
					});
		}
		addShader(SHADER_OUTPUT_DEPTH, _shader_1);
	};
}

void Scene::setupRootScene()
{
	preloadRenderPass();
	// 预加载Shader
	preloadShader();
	prebindData();
}

Core::Resource::Camera* Scene::getMainCamera()
{
	if (m_main_camera_count > 0)
		return p_main_camera;
}

void Scene::onResize(const VkExtent3D& _canvas_size)
{
	for (const auto& [_name, _cam] : p_camera) {
		if (_cam->isMain())
			_cam->onResize(_canvas_size);
	}
}

Core::Resource::Scene* Scene::removeCamera(const std::string& _name)
{
	auto ite = p_camera.find(_name);
	if (ite != p_camera.end()) {
		delete ite->second;
		p_camera.erase(ite);
		p_world_scenes.erase(p_world_scenes.find(_name));
	}
	return this;
}

Core::Resource::Scene* Scene::addCamera(const VkExtent3D& _canvas_size, const std::string& _name /*= "Camera"*/)
{
	Camera* _main_camera = new Camera(_canvas_size, _name);
	if (m_main_camera_count == 0) {
		_main_camera->setMainCamera();
		p_main_camera = _main_camera;
		m_main_camera_count++;
		// 设置MainCamera的depth 输出
		auto _sub_cam = addDepthSubCamera(_canvas_size, "Main Depth Camera");
		_sub_cam->setPosAndTarget(_main_camera);
		_main_camera->setSubCamera(_sub_cam);
	}
	p_camera[_main_camera->getSceneName()] = _main_camera;
	p_world_scenes[_main_camera->getSceneName()] = _main_camera;
	return this;
}

Core::Resource::Scene* Scene::getRootScene()
{
	return p_root_scene;
}

void Scene::deserializeToObj(Json::Value& root)
{

}

Json::Value Scene::serializeToJSON()
{
	return Json::Value();
}

Core::Resource::Scene* Scene::push_back(Scene* _p)
{
	_p->p_next = nullptr;
	auto last = p_root_scene->p_last;
	// 最后一个不为空
	if (last) {
		last->p_next = _p;
		_p->p_previous = last;
	}
	p_root_scene->p_last = _p;
	return this;
}

Scene::Scene(const VkExtent3D& _canvas_size, const std::string& _name /*= "Scene"*/)
	:ZResource(), SceneObject(_name), m_canvas_size(_canvas_size)
{
	p_current_choose_obj = nullptr;

	// 如果无根节点，则创建
	if (!p_root_scene) {
		p_root_scene = new Scene();
	}
	// 添加至链表末端 
	p_root_scene->push_back(this);

	// 添加摄像机
	{
		// 主摄像
		addCamera(_canvas_size, "Main Camera");
		auto _cam = getMainCamera();
		_cam->getProperty()->m_rotate_R = 15.0f;
	}
	// 添加立方体贴图卷积用摄像
	{
		auto _tex = new TextureEXT(
			{ 32,32,1 },
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6,
			VK_SAMPLE_COUNT_1_BIT,
			"Convolution CubeTexture"
		);
		VkExtent3D _size = { 32,32,1 };
		p_convolution_camera = addCubeSubCamera(_size,"Convolution Sub Camera");
		p_convolution_camera
			->addAttachment(COLOR_ATTACHMENT, _tex)
			->createCubeFramebuffer()
			->createCommandBuffer()
			->endInitialize();
		p_convolution_camera->onUpdate();

	}
	// 添加hdri转立方体贴图用摄像
	{
		VkExtent3D _size = { 512,512,1 };
		auto _tex = new TextureEXT(
			_size,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6,
			VK_SAMPLE_COUNT_1_BIT,
			"HDRI To CubeTexture"
		);
		p_hdri_to_cube_camera = addCubeSubCamera(_size, "HDRI to CubeMap Sub Camera");
		p_hdri_to_cube_camera
			->addAttachment(COLOR_ATTACHMENT, _tex)
			->createCommandBuffer()
			->createCubeFramebuffer()
			->endInitialize();
		p_hdri_to_cube_camera->onUpdate();

	}
	// 添加平面贴图保存brdf积分
	{
		VkExtent3D _size = { 512,512,1 };
		auto _tex = new TextureEXT(
			_size,
			VK_FORMAT_R8G8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			1,VK_SAMPLE_COUNT_1_BIT,
			"BrdfLUT Texture"
		);
		_tex->setVkSampler(ThirdParty::Sampler::g_all_samplers[SAMPLER_REPEAT]->getVkSampler());
		p_brdf_camera = addSubCamera(_size, "BrdfLUT Sub Camera");
		p_brdf_camera->getHidden() = true;
		p_brdf_camera
			->setRRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_RG_NODEPTH))
			->addAttachment(TEXTURE_TYPE::COLOR_ATTACHMENT, _tex)
			->createCommandBuffer()
			->createFramebuffer()
			->endInitialize();
		p_brdf_camera->onUpdate();
	}
	// 添加预滤波环境贴图
	{
		VkExtent3D _size = { 128,128,1 };
		auto _tex = new TextureEXT(
			_size,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6,
			VK_SAMPLE_COUNT_1_BIT,
			"Prefilter CubeTexture",
			roughnessLog
		);
		_tex->setVkSampler(ThirdParty::Sampler::g_all_samplers[SAMPLER_REPEAT]->getVkSampler());
		p_prefilter_camera = addSubCamera(_size, "Prefilter Sub Camera");
		p_prefilter_camera->getHidden() = true;
		p_prefilter_camera
			->setRRenderPass(Core::Resource::RRenderPass::getRRenderPass(RENDER_PASS_NODEPTH))
			->addAttachment(TEXTURE_TYPE::COLOR_ATTACHMENT, _tex)
			->createCommandBuffer()
			->createCubeFramebuffer(0, roughnessLog)
			->endInitialize();
		p_prefilter_camera->onUpdate();
	}
	// 测试物体
	{
		// 添加全局光
		{
			// addAmbientLight(glm::vec3(1),0.2);
			// X - 上， Y - 右
			addDirectionLight(glm::vec3(-60.f, -130.f, 0), glm::vec4(1, 1, 1, 0.8));
		}
		// 相机位置
		auto _cam_prop = p_main_camera->getProperty();
		_cam_prop->update();
		_cam_prop->m_wheel = -10.0f;
		_cam_prop->m_axis_rotate = { -45.0f, -45.0f, 0.0f };
		_cam_prop->m_zFar = 200.0f;
		// 平面
		auto _plane = addShape(PLANE, glm::vec3(0, 0.0f, 0));
		_plane->getProperty()->m_scale = { 10,1,10 };
		// 立方体房间(点光源测试)
		// auto _cube_room = addShape(CUBE, glm::vec3(0, 0, 0));
		// _cube_room->getProperty()->m_scale = { 200,200,200 };
		// 立方体& 球体
		uint32_t _shape_count = 2;
		for (int i = 0; i < _shape_count; i++) {
			auto _cube = addShape(SCENE_TYPE((int)CUBE + (i % 2)), glm::vec3(0, 0.5f, 0));
			_cube->getProperty()->m_vec3_pos += glm::vec3(i * 1.5f);
		}
	}
	setZResourceProperty(p_property);
	initTemplate();

	// 生成brdf积分图
	{
		if (p_brdf_camera->isReady()) {
			auto _cmd = p_brdf_camera->onRenderStartImmediately();
			auto _mat = p_materials[getShader(SHADER_BRDF)];
			auto _func = getDrawFunc(getShader(SHADER_BRDF));
			Core::Resource::draw_shape(PLANE, this, _mat, _cmd, _func, p_brdf_camera);
			p_brdf_camera->onRenderEndImmediately(_cmd);
		}
		
	}
	// 保存当前默认的天空盒
	auto _tex = TextureEXT::getTextureEXT("default-skybox-hdri");
	addSkybox(_tex->getPaths()[0]->m_path,_tex);
}

Scene::Scene()
	:ZResource(), SceneObject("Root Scene")
{
	p_current_choose_obj = nullptr;
	// 如果是根节点
	if (!p_root_scene) {
		p_root_scene = this;
		p_root_scene->m_list_index = 0;
		p_root_scene->p_next = NULL;
		p_root_scene->p_previous = NULL;
		p_root_scene->p_last = this;
		m_is_root_scene = true;
		setupRootScene();
	}
	else {
		LOG_ERROR("Failed to create root scene.It is already created!");
	}
	setZResourceProperty(p_property);
	// 设置Shader空间下的列表
	Shader::p_shaders = &p_shaders;
}
