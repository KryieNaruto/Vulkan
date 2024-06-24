#pragma once
#include <Resource/resource_global.h>
#include "include/sub/sub_include.h"

#define MAIN_CAMERA "Main Camera"

namespace Core::Resource {

    //--------------------------------------------
    // Resource�ļ���,����Resource����Ⱦ���ߣ��Ծ���Resource����
    // ��ͷ�������ʽ�洢���г�����������Root, ��������Ԥ����shader
    // ��Ҫ��Core::Core_Pool�е�ȫ�ֱ�������Scene����,Core::Core_Pool����Scene
    //--------------------------------------------

class ENGINE_API_SCENE Scene :
	public SceneObject,
    public ZResource
{
public:
    //-------------------------------
    // ��̬��Ա
    //-------------------------------
    static Scene* p_root_scene;              // ������
    static std::unordered_map<std::string, Shader*> p_shaders;                  // ���о���Ԥ���ص�shader
    static std::unordered_map<Shader*, Material*> p_materials;                  // ���о���Ԥ���ص�shader
	static std::unordered_map<Shader*, Shader_Data_Binding_Func> p_draw_func;
    static std::unordered_map<TextureEXT*, TextureEXT*> p_convolution_textures; // �����Ѿ�������Ļ�����ͼ
    static std::unordered_map<TextureEXT*, TextureEXT*> p_prefilter_textures;   // Ԥ�˲�����ͼ��key=������ͼ,value = LOD�˲�������ͼ
    static TextureEXT* p_brdf_texture; // brdf����ͼ
    static std::vector<std::string> m_hdri_texture_paths;
    static std::unordered_map<std::string, TextureEXT*>p_hdri_textures;
    static std::unordered_map<TextureEXT*, TextureEXT*> p_cube_textures;   // ת�������������ͼ
public:
    // �ճ���������һ��Main Camera; 
    // �����Root�ڵ㣬�򲻻ᴴ��Camera
    Scene();    // ���ڵ�
    Scene(const VkExtent3D& _canvas_size, const std::string& _name = "Scene");
    virtual ~Scene();
    // ���л�����
	virtual Json::Value serializeToJSON();
    // ���泡��
	virtual void deserializeToObj(Json::Value& root);
	virtual void propertyEditor() override;
    //----------------------------
    // �������
    //----------------------------
    // ��ӵ����
    Scene* push_back(Scene* _p);
    Scene* getNext() { return p_next; }
    Scene* getPrevious() { return p_previous; }
    Scene* getLast();
    static Scene* getRootScene();

    //--------------------------------
    // ��Դ����
    //--------------------------------
    static Shader* getShader(const std::string& _name);
    static void addShader(const std::string& _name, Shader* _shader);
    static const std::unordered_map<std::string, Shader*>& getShaders();
    static const std::unordered_map<Shader*, Material*>& getMaterials();
    // ����Material
    Scene* reloadMaterial(Shader* _shader);
    //--------------------------------
    // �¼�
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
	uint32_t m_skybox_shape = SPHERE; // ��պ���ʽ
	uint32_t m_skybox_shape_last = SPHERE; // ��պ���ʽ
    uint32_t m_skybox_hdri_index = 0;   // ��պ�hdri����
	// �Ƿ���ʾ2d����
	bool m_show_2d_mesh = true;
    // �Ƿ�ʹ�ù���
    bool m_show_light = true;
    // �Ƿ�ʹ����պ�
    bool m_show_skybox = true;
    bool m_is_debug = false;
    // ����һ����Ⱦ
    bool m_is_skip = false;
    // ʹ�÷��նȻ�����պ�
    bool m_use_convolution = false;
    // PBR �ֲڶȷּ�
    const int roughnessLog = 5;
    // ������С
    VkExtent3D m_canvas_size = { 800,600 };

	uint32_t m_list_index;          // ��������λ�ã���������
	Scene* p_next;                  // ��һ��
	Scene* p_previous;              // ��һ��
	Scene* p_last;                  // ���һ�� [��p_root_scene] ��Ч
    bool m_is_root_scene = false;

    // world ������Scene_obj
    SceneObject* p_current_choose_obj;
    std::unordered_map<std::string, SceneObject*> p_world_scenes;
    //--------------------------------------------
    std::unordered_map<std::string, Model*> p_models;   // ģ��
    std::unordered_map<std::string, Shape*> p_shapes;    // ����ģ��
    std::unordered_map<Shader*, std::vector<Material*>> p_model_materials;  // ģ�Ͳ���
    std::unordered_map<std::string, Light*> p_lights;  // ��Դ
    //-------------------------------------------- ����
    uint32_t m_main_camera_count = 0;
    Camera* p_main_camera = nullptr;
    SubCamera* p_convolution_camera = nullptr;
    SubCamera* p_prefilter_camera = nullptr;
    SubCamera* p_brdf_camera = nullptr;
    SubCamera* p_hdri_to_cube_camera = nullptr;
	std::unordered_map<std::string, Camera*> p_camera;  // ����

	inline void init_0()
	{
		this->m_json_title = "Scene";
		this->p_id_generate->init(RESOURCE_TYPE::SCENE);
		this->m_uuid = this->p_id_generate->generateUUID();
	}
    // �������ڵ㣬Ԥ����Shader��󶨺���
    void setupRootScene();
    void preloadRenderPass();
    void preloadShader();
    void prebindData();
public:
	//-------------------------------------------- ���ݰ�
    static void addDrawFunc(Shader* _shader, Shader_Data_Binding_Func _draw_func);
    static void delDrawFunc(Shader* _shader);
    static Shader_Data_Binding_Func getDrawFunc(Shader* _shader);
    //---------------------------------------- ����
    // ������������MainCamera ,�����õ�ǰCmaeraΪMainCamera
    Scene* addCamera(const VkExtent3D& _canvas_size, const std::string& _name = "Camera");
    Scene* removeCamera(const std::string& _name);
    SubCamera* addSubCamera(const VkExtent3D& _canvas_size, const std::string& _name = "SubCamera");
    // ��ӹ�Դ������
    SubCamera* addDepthSubCamera(const VkExtent3D& _canvas_size, const std::string& _name = "SubCamera");
    // ��������ͼ���������������ͼ
    SubCamera* addCubeSubCamera(const VkExtent3D& _canvas_size,const std::string& _name = "SubCamera");
    // �����ǰ������ͼ,������
    Scene* convolutionSkybox();
    Camera* getMainCamera();
    //---------------------------------------- ��Դ
    // ���ƽ�й�
    Scene* addLight(Light* _light);
    Scene* addDirectionLight(const glm::vec3& _direction,const glm::vec4& _light_color);
    Scene* addAmbientLight(const glm::vec3& _light_color, float _strength);
    Scene* addPointLight(const glm::vec3& _pos,const glm::vec4& _light_color);
    Scene* addSpotLight(const glm::vec3& _pos,const glm::vec4& _light_color);
    Scene* removeLight(const std::string& _name);
    std::unordered_map<std::string, Light*>& getLights() { return p_lights; }

    //---------------------------------------- ģ��
    Scene* addModel(Model* _model);
    Scene* loadModelFromFile(const std::string& _path);
    SceneObject* getCurrentChooseSceneObject() { return p_current_choose_obj; }
    void chooseSceneObject(SceneObject* _obj) { p_current_choose_obj = _obj; }
    std::unordered_map<std::string, Model*>& getModels() { return p_models; }
    std::unordered_map<std::string, SceneObject*>& getWorldSceneObjects() { return p_world_scenes; }
    std::unordered_map<Shader*, std::vector<Material*>>& getModelMaterials() { return p_model_materials; }
    //---------------------------------------- ����ģ��
    Shape* addShape(SCENE_TYPE _shape_type,const glm::vec3& _pos = glm::vec3(0));
    //---------------------------------------- ��պ�
    uint32_t& getSkyboxType() { return m_skybox_shape; }
    void setSkyboxType(uint32_t _type) { m_skybox_shape = _type; }
    void setSkyboxIndex(uint32_t _index) { m_skybox_hdri_index = _index; }
    Scene* addSkybox(const std::string& _path, TextureEXT* _tex);
    // ��ȡ��ǰ���ն���ͼ
    inline TextureEXT* getCurrentSkyboxConvolutionTextureEXT() {
        return getSkyboxHDRITextureCube_Convolution();
    }
    TextureEXT* getSkyboxHDRITexture();
    TextureEXT* getSkyboxHDRITextureCube();
    TextureEXT* getSkyboxHDRITextureCube_Prefilter();
    TextureEXT* getSkyboxHDRITextureCube_Convolution();


    // ͨ�� ZResource �̳�
    void initTemplate() override;

};
}

