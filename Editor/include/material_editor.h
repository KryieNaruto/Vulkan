#pragma once
#include <Scene/scene_include.h>
#include <ThirdPartyManager/third_party_manager_global.h>
#include "base_editor.h"
#include "editor_global_func.h"

using namespace Core::Resource;
namespace imgui = ThirdParty::imgui;
using namespace imgui;
namespace Editor {

	struct MaterialDelegate :public imgui::Delegate
	{
		virtual ~MaterialDelegate();
		// 节点
		std::vector<imgui::Node*> p_nodes;
		// 链接
		std::vector<imgui::Link*> p_links;
		// 显示模板
		std::unordered_map<Template*,uint32_t> p_templates;
		std::unordered_map<uint32_t,Template*> p_templates_indexed;
		// 链接
		virtual bool allowedLink(NodeIndex from, NodeIndex to);

		// 删除节点
		void delNode(Node* _node);
		// 添加节点
		void addNode(Node* _node);
		// 选择节点
		virtual void selectNode(NodeIndex nodeIndex, bool selected);
		// 移动节点
		virtual void moveSelectedNodes(const ImVec2 delta);

		// 添加链接
		virtual void addLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex);
		// 删除链接
		virtual void delLink(LinkIndex linkIndex);

		// 获取模板索引
		uint32_t getTemplateIndex(Template* _temp);
		// 添加模板
		void addTemplate(Template* _temp);
		// 删除模板
		void delTemplate(Template* _temp);
		void delTemplate(TemplateIndex _index);
		// 获取模板数量
		virtual const size_t getTemplateCount();
		// 获取模板
		virtual Template* getTemplate(TemplateIndex index);

		// 获取节点数量
		virtual const size_t getNodeCount();
		// 获取节点, 根据template设置宽高，默认宽高200
		virtual Node* getNode(NodeIndex index);

		// 获取连接数量
		virtual const size_t getLinkCount();
		// 获取连接
		virtual Link* getLink(LinkIndex index);
	};

    // 结合Shader Material Scene RenderPass Pipeline
    // 以Scene为单位
    class MaterialEditor:
        public ZEditor
    {
    public:
        MaterialEditor();
        virtual ~MaterialEditor();
		void onRender() override;
		void onUpdate() override;
        virtual void onInput();
    	// 处理编辑，例如 添加RenderPass、Shader、Texture等
    	void handleEditor();
    	// 处理加载文件
    	void handleLoading();
        Scene* p_scene = nullptr;		// 场景
    private:
        //--------------------------------------------
        // 详情图
        bool m_is_active_detail = false;
		// 是否激活
		bool m_is_active = false;
		VkExtent3D m_canvas_size = { 200, 200 ,1 };
        imgui::Options* p_options = nullptr;				// 选项
        imgui::ViewState* p_view_state = nullptr;			// 视图信息
        MaterialDelegate* p_material_delegate = nullptr;	// 材质节点信息
        //--------------------------------------------
		// 标志位
    	typedef enum MATERIAL_EDITOR_CREATE_TYPE {
    		_Pipeline,		// 管线
    		_PipelineLayout,// 管线布局
    		_Shader,		// Shader
    		_RenderPass,	// RenderPass
    		_File,			// 文件
    		_Material,		// 材质
    		_Texture,		// 纹理
			_Math,			// 计算节点
			_Data = 0x10,	// 数据
			_Data_Camera,	// 摄像数据
			_Data_Light,	// 光源数据
			_Data_Model,	// 模型数据
			_Data_Scene,	// 场景数据
			_Common_Struct,	// 通用结构体
			_Data_Struct,	// 通用结构体
    	}CreateType;
    	bool m_create_flags[256] = {false};					// 创建
    	//-------------------------------------------
    	// 加载文件ing
    	std::string m_file_path_loading = "";
		// 当前创建中的Material
		Core::Resource::Material* p_material_in_scene = nullptr;	// 即将进入Scene的Mat
		std::set<Core::Resource::Material*> p_materials_creating;
		// 当前创建中的Shader
		std::set<Core::Resource::Shader*> p_shaders_creating;
		// 当前创建中的Texture
		std::set<ThirdParty::TextureEXT*> p_texture_ext_createing;
		// 渲染preview camera
		std::unordered_map<Core::Resource::Material*, Core::Resource::Camera*> p_cameras_createting;
		// Material Ubo Struct
		Core::CommonStruct* p_common_struct_new = nullptr;
		Core::Resource::RLightProperty* p_light_property_new = nullptr;
		std::set<Core::CommonStruct*> p_common_structs_createing;

    	// 创建新节点并保存
    	void createNode(ZResource* _obj);
    	void createNode(Core::FileInfo* _obj);
    	void createNode(ThirdParty::vkObject* _obj);
		void createNode(Core::Resource::RObjectProperty* _prop);
		void createNode(Core::CommonStruct* _struct);
    	// 返回标志位，并将标志位置为False
    	bool checkCreateFlag(bool& _flag);
		// 处理当前创建中的 ---
		void handleLink();	// 处理链接，将数据之间互相联通，input.p_data = output.p_data;
		bool handleAllShaders();
		void handleAllMaterials();
		void handleAllTextures();
		void handleAllFunc();	// 处理计算方式，例如Camera.proj * Camera.View等, 处理Material 的额外input
		void handleAllGenerate();	// 处理生成材质

		void Debug();
    };

}
