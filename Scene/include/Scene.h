#pragma once
#include <Resource/resource_global.h>
#include "include/sub/sub_include.h"

#define MAIN_CAMERA "Main Camera"

namespace Core::Resource {

    //--------------------------------------------
    // Resource的集合,解耦Resource和渲染管线，仍旧是Resource类型
    // 有头链表的形式存储所有场景，根场景Root, 保存所有预加载shader
    // 需要将Core::Core_Pool中的全局变量交由Scene保管,Core::Core_Pool保管Scene
    //--------------------------------------------

class ENGINE_API_SCENE Scene :
	public SceneObject,
    public ZResource
{
public:
    //-------------------------------
    // 静态成员
    //-------------------------------
    static Scene* p_root_scene;              // 根场景
    static std::unordered_map<std::string, Shader*> p_shaders;                  // 所有经过预加载的shader
    static std::unordered_map<Shader*, Material*> p_materials;                  // 所有经过预加载的shader
	static std::unordered_map<Shader*, Shader_Data_Binding_Func> p_draw_func;
    static std::unordered_map<TextureEXT*, TextureEXT*> p_convolution_textures; // 所有已经卷积过的环境贴图
    static std::unordered_map<TextureEXT*, TextureEXT*> p_prefilter_textures;   // 预滤波环境图，key=环境贴图,value = LOD滤波环境贴图
    static TextureEXT* p_brdf_texture; // brdf积分图
    static std::vector<std::string> m_hdri_texture_paths;
    static std::unordered_map<std::string, TextureEXT*>p_hdri_textures;
    static std::unordered_map<TextureEXT*, TextureEXT*> p_cube_textures;   // 转化后的立方体贴图
public:
    // 空场景，包含一个Main Camera; 
    // 如果是Root节点，则不会创建Camera
    Scene();    // 根节点
    Scene(const VkExtent3D& _canvas_size, const std::string& _name = "Scene");
    virtual ~Scene();
    // 序列化场景
	virtual Json::Value serializeToJSON();
    // 保存场景
	virtual void deserializeToObj(Json::Value& root);
	virtual void propertyEditor() override;
    //----------------------------
    // 链表操作
    //----------------------------
    // 添加到最后
    Scene* push_back(Scene* _p);
    Scene* getNext() { return p_next; }
    Scene* getPrevious() { return p_previous; }
    Scene* getLast();
    static Scene* getRootScene();

    //--------------------------------
    // 资源检索
    //--------------------------------
    static Shader* getShader(const std::string& _name);
    static void addShader(const std::string& _name, Shader* _shader);
    static const std::unordered_map<std::string, Shader*>& getShaders();
    static const std::unordered_map<Shader*, Material*>& getMaterials();
    // 重载Material
    Scene* reloadMaterial(Shader* _shader);
    //--------------------------------
    // 事件
    //--------------------------------
    virtual void onResize(const VkExtent3D& _canvas_size);
    virtual void onRender();
    virtual void onUpdate();
public:
    inline void skipOnce() { m_is_skip = true; }
    inline bool& getShow_2d_mesh() { return m_show_2d_mesh; }
    inline bool& getShow_light() { return m_show_light; }
    inline bool& getShow_skybox() { return m_show_skybox; }
    inline bool& getIs_Debug() { return m_is_debug; }
    inline bool& isRootScene() { return m_is_root_scene; }
    inline bool& isSkip() { return m_is_skip; }
    inline bool& isUseConvolution() { return m_use_convolution; }
    inline uint32_t& getSkyboxIndex() { return m_skybox_shape_index; }
private:
    uint32_t m_skybox_shape_index = 1;  // 0:CUBE, 1:SPHERE
	uint32_t m_skybox_shape = SPHERE; // 天空盒样式
	uint32_t m_skybox_shape_last = SPHERE; // 天空盒样式
    uint32_t m_skybox_hdri_index = 0;   // 天空盒hdri索引
	// 是否显示2d网格
	bool m_show_2d_mesh = true;
    // 是否使用光照
    bool m_show_light = true;
    // 是否使用天空盒
    bool m_show_skybox = true;
    bool m_is_debug = false;
    // 跳过一次渲染
    bool m_is_skip = false;
    // 使用辐照度环境天空盒
    bool m_use_convolution = false;
    // PBR 粗糙度分级
    const int roughnessLog = 5;
    // 画布大小
    VkExtent3D m_canvas_size = { 800,600 };

	uint32_t m_list_index;          // 处于链表位置，方便索引
	Scene* p_next;                  // 上一个
	Scene* p_previous;              // 下一个
	Scene* p_last;                  // 最后一个 [仅p_root_scene] 有效
    bool m_is_root_scene = false;

    // world 下所有Scene_obj
    SceneObject* p_current_choose_obj;
    std::unordered_map<std::string, SceneObject*> p_world_scenes;
    //--------------------------------------------
    std::unordered_map<std::string, Model*> p_models;   // 模型
    std::unordered_map<std::string, Shape*> p_shapes;    // 简易模型
    std::unordered_map<Shader*, std::vector<Material*>> p_model_materials;  // 模型材质
    std::unordered_map<std::string, Light*> p_lights;  // 光源
    //-------------------------------------------- 摄像
    uint32_t m_main_camera_count = 0;
    Camera* p_main_camera = nullptr;
    SubCamera* p_convolution_camera = nullptr;
    SubCamera* p_prefilter_camera = nullptr;
    SubCamera* p_brdf_camera = nullptr;
    SubCamera* p_hdri_to_cube_camera = nullptr;
	std::unordered_map<std::string, Camera*> p_camera;  // 摄像

	inline void init_0()
	{
		this->m_json_title = "Scene";
		this->p_id_generate->init(RESOURCE_TYPE::SCENE);
		this->m_uuid = this->p_id_generate->generateUUID();
	}
    // 创建根节点，预加载Shader与绑定函数
    void setupRootScene();
    void preloadRenderPass();
    void preloadShader();
    void prebindData();
public:
	//-------------------------------------------- 数据绑定
    static void addDrawFunc(Shader* _shader, Shader_Data_Binding_Func _draw_func);
    static void delDrawFunc(Shader* _shader);
    static Shader_Data_Binding_Func getDrawFunc(Shader* _shader);
    //---------------------------------------- 摄像
    // 添加摄像，如果无MainCamera ,则设置当前Cmaera为MainCamera
    Scene* addCamera(const VkExtent3D& _canvas_size, const std::string& _name = "Camera");
    Scene* removeCamera(const std::string& _name);
    SubCamera* addSubCamera(const VkExtent3D& _canvas_size, const std::string& _name = "SubCamera");
    // 添加光源深度相机
    SubCamera* addDepthSubCamera(const VkExtent3D& _canvas_size, const std::string& _name = "SubCamera");
    // 立方体贴图摄像，输出立方体贴图
    SubCamera* addCubeSubCamera(const VkExtent3D& _canvas_size,const std::string& _name = "SubCamera");
    // 卷积当前环境贴图,并保存
    Scene* convolutionSkybox();
    Camera* getMainCamera();
    //---------------------------------------- 光源
    // 添加平行光
    Scene* addLight(Light* _light);
    Scene* addDirectionLight(const glm::vec3& _direction,const glm::vec4& _light_color);
    Scene* addAmbientLight(const glm::vec3& _light_color, float _strength);
    Scene* addPointLight(const glm::vec3& _pos,const glm::vec4& _light_color);
    Scene* addSpotLight(const glm::vec3& _pos,const glm::vec4& _light_color);
    Scene* removeLight(const std::string& _name);
    std::unordered_map<std::string, Light*>& getLights() { return p_lights; }

    //---------------------------------------- 模型
    Scene* addModel(Model* _model);
    Scene* loadModelFromFile(const std::string& _path);
    SceneObject* getCurrentChooseSceneObject() { return p_current_choose_obj; }
    void chooseSceneObject(SceneObject* _obj) { p_current_choose_obj = _obj; }
    std::unordered_map<std::string, Model*>& getModels() { return p_models; }
    std::unordered_map<std::string, SceneObject*>& getWorldSceneObjects() { return p_world_scenes; }
    std::unordered_map<Shader*, std::vector<Material*>>& getModelMaterials() { return p_model_materials; }
    //---------------------------------------- 简易模型
    Shape* addShape(SCENE_TYPE _shape_type,const glm::vec3& _pos = glm::vec3(0));
    //---------------------------------------- 天空盒
    uint32_t& getSkyboxType() { return m_skybox_shape; }
    void setSkyboxType(uint32_t _type) { m_skybox_shape = _type; }
    void setSkyboxIndex(uint32_t _index) { m_skybox_hdri_index = _index; }
    Scene* addSkybox(const std::string& _path, TextureEXT* _tex);
    // 获取当前辐照度贴图
    inline TextureEXT* getCurrentSkyboxConvolutionTextureEXT() {
        return getSkyboxHDRITextureCube_Convolution();
    }
    TextureEXT* getSkyboxHDRITexture();
    TextureEXT* getSkyboxHDRITextureCube();
    TextureEXT* getSkyboxHDRITextureCube_Prefilter();
    TextureEXT* getSkyboxHDRITextureCube_Convolution();


    // 通过 ZResource 继承
    void initTemplate() override;

};
}

