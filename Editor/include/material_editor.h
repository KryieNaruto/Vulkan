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
		// �ڵ�
		std::vector<imgui::Node*> p_nodes;
		// ����
		std::vector<imgui::Link*> p_links;
		// ��ʾģ��
		std::unordered_map<Template*,uint32_t> p_templates;
		std::unordered_map<uint32_t,Template*> p_templates_indexed;
		// ����
		virtual bool allowedLink(NodeIndex from, NodeIndex to);

		// ɾ���ڵ�
		void delNode(Node* _node);
		// ��ӽڵ�
		void addNode(Node* _node);
		// ѡ��ڵ�
		virtual void selectNode(NodeIndex nodeIndex, bool selected);
		// �ƶ��ڵ�
		virtual void moveSelectedNodes(const ImVec2 delta);

		// �������
		virtual void addLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex);
		// ɾ������
		virtual void delLink(LinkIndex linkIndex);

		// ��ȡģ������
		uint32_t getTemplateIndex(Template* _temp);
		// ���ģ��
		void addTemplate(Template* _temp);
		// ɾ��ģ��
		void delTemplate(Template* _temp);
		void delTemplate(TemplateIndex _index);
		// ��ȡģ������
		virtual const size_t getTemplateCount();
		// ��ȡģ��
		virtual Template* getTemplate(TemplateIndex index);

		// ��ȡ�ڵ�����
		virtual const size_t getNodeCount();
		// ��ȡ�ڵ�, ����template���ÿ�ߣ�Ĭ�Ͽ��200
		virtual Node* getNode(NodeIndex index);

		// ��ȡ��������
		virtual const size_t getLinkCount();
		// ��ȡ����
		virtual Link* getLink(LinkIndex index);
	};

    // ���Shader Material Scene RenderPass Pipeline
    // ��SceneΪ��λ
    class MaterialEditor:
        public ZEditor
    {
    public:
        MaterialEditor();
        virtual ~MaterialEditor();
		void onRender() override;
		void onUpdate() override;
        virtual void onInput();
    	// ����༭������ ���RenderPass��Shader��Texture��
    	void handleEditor();
    	// ��������ļ�
    	void handleLoading();
        Scene* p_scene = nullptr;		// ����
    private:
        //--------------------------------------------
        // ����ͼ
        bool m_is_active_detail = false;
		// �Ƿ񼤻�
		bool m_is_active = false;
		VkExtent3D m_canvas_size = { 200, 200 ,1 };
        imgui::Options* p_options = nullptr;				// ѡ��
        imgui::ViewState* p_view_state = nullptr;			// ��ͼ��Ϣ
        MaterialDelegate* p_material_delegate = nullptr;	// ���ʽڵ���Ϣ
        //--------------------------------------------
		// ��־λ
    	typedef enum MATERIAL_EDITOR_CREATE_TYPE {
    		_Pipeline,		// ����
    		_PipelineLayout,// ���߲���
    		_Shader,		// Shader
    		_RenderPass,	// RenderPass
    		_File,			// �ļ�
    		_Material,		// ����
    		_Texture,		// ����
			_Math,			// ����ڵ�
			_Data = 0x10,	// ����
			_Data_Camera,	// ��������
			_Data_Light,	// ��Դ����
			_Data_Model,	// ģ������
			_Data_Scene,	// ��������
			_Common_Struct,	// ͨ�ýṹ��
			_Data_Struct,	// ͨ�ýṹ��
    	}CreateType;
    	bool m_create_flags[256] = {false};					// ����
    	//-------------------------------------------
    	// �����ļ�ing
    	std::string m_file_path_loading = "";
		// ��ǰ�����е�Material
		Core::Resource::Material* p_material_in_scene = nullptr;	// ��������Scene��Mat
		std::set<Core::Resource::Material*> p_materials_creating;
		// ��ǰ�����е�Shader
		std::set<Core::Resource::Shader*> p_shaders_creating;
		// ��ǰ�����е�Texture
		std::set<ThirdParty::TextureEXT*> p_texture_ext_createing;
		// ��Ⱦpreview camera
		std::unordered_map<Core::Resource::Material*, Core::Resource::Camera*> p_cameras_createting;
		// Material Ubo Struct
		Core::CommonStruct* p_common_struct_new = nullptr;
		Core::Resource::RLightProperty* p_light_property_new = nullptr;
		std::set<Core::CommonStruct*> p_common_structs_createing;

    	// �����½ڵ㲢����
    	void createNode(ZResource* _obj);
    	void createNode(Core::FileInfo* _obj);
    	void createNode(ThirdParty::vkObject* _obj);
		void createNode(Core::Resource::RObjectProperty* _prop);
		void createNode(Core::CommonStruct* _struct);
    	// ���ر�־λ��������־λ��ΪFalse
    	bool checkCreateFlag(bool& _flag);
		// ����ǰ�����е� ---
		void handleLink();	// �������ӣ�������֮�以����ͨ��input.p_data = output.p_data;
		bool handleAllShaders();
		void handleAllMaterials();
		void handleAllTextures();
		void handleAllFunc();	// ������㷽ʽ������Camera.proj * Camera.View��, ����Material �Ķ���input
		void handleAllGenerate();	// �������ɲ���

		void Debug();
    };

}
