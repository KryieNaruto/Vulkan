#pragma once
#include <ThirdParty/imgui/imgui.h>
#include "ThirdPartyManager/core/info/include/application_window_info.h"
#include <string>
#include <functional>
namespace ThirdParty::imgui {

	typedef int NodeIndex;
	typedef int SlotIndex;
	typedef int LinkIndex;
	typedef int TemplateIndex;

	enum NodeOperation
	{
		NO_None,
		NO_EditingLink,
		NO_QuadSelecting,
		NO_MovingNodes,
		NO_EditInput,
		NO_PanView,
	};

	struct ENGINE_API_THIRDPARTY Options {
		ImVec2 m_slot_size = { 200.0f,25.0f };						// ��۴�С
		ImVec2 m_slot_name_padding = { 30,0 };						// Slot ����ƫ��
		ImVec2 m_canvas_start_pos = { 0,0 };						// ������ʼλ��
		ImU32 m_bg_color = IM_COL32(40, 40, 40, 255);				// ��������
		ImU32 m_grid_color_0 = IM_COL32(0, 0, 0, 60);				// ϸ����ɫ
		ImU32 m_grid_color_1 = IM_COL32(0, 0, 0, 160);				// ������ɫ
		ImU32 m_text_color = IM_COL32(255, 255, 255, 255);			// ������ɫ
		ImU32 m_select_border_color = IM_COL32(255, 130, 30, 255);	// ѡ�б߿���ɫ
		ImU32 m_quad_selection = IM_COL32(255, 32, 32, 64);			// ѡ����ɫ
		ImU32 m_quad_selection_border = IM_COL32(255, 32, 32, 64);	// ѡ���߿���ɫ
		float m_point_size = 8.0f;									// Point �뾶
		float m_point_active_size = 16.0f;							// Point Hover�뾶
		float m_grid_size = 64.0f;									// �����С
		float m_zoom_ratio = 0.1f;									// ������������
		float m_zoom_lerp_factor = 0.25f;							// ��������
		float m_min_zoom = 0.2f, m_max_zoom = 2.3f;					// ���ŷ�Χ
		float m_border_thickness = 6.f;								// �߿��ϸ
		float m_select_border_thickness = 6.f;						// �߿�ѡ�д�ϸ
		float m_rouding = 3.f;										// Բ��
	};

    struct DebugInfo {
        int m_select_node_index = -1;
        int m_hover_node_index = -1;
        int m_hover_slot_index = -1;
    };
	struct ENGINE_API_THIRDPARTY ViewState {
        ImVec2 m_mouse_position = {0,0};
        ImVec2 m_mouse_position_in_canvas = {0,0};
		ImVec2 m_window_position = {0,0};		// ����λ��
		ImVec2 m_canvas_size = { 0,0 };			// ������С
		ImVec2 m_position = { 0,0 };			// ����λ��
		ImVec2 m_cursor_screen_pos = { 0,0 };	// ��һ���λ��
		ImVec2 m_current_slot_start_pos = { 0,0 };// ��ǰSlot��ʼλ�ã����ڼ���Slot�߶�
		ImVec2 m_imgui_start = { 0,0 };			// ��ǰimgui�ؼ���ʼλ��
		float m_factor = 1.0f;					// ��ǰzoom factor
		float m_factor_target = 1.0f;			// Ŀ��zoom factor, ʹ��Options::m_zoom_lerp_factor��ֵ
		bool m_is_hovered = false;				// �Ƿ�hover
		bool m_is_active = false;				// �Ƿ������
        DebugInfo m_debug_info;
	};

	// Slot ��ʽ
	struct ENGINE_API_THIRDPARTY SlotTemplate {
		const char* m_name = nullptr;
		const char* m_name_2 = nullptr;	// �����ƣ����ڱ�ʶ�ض����ݣ�����ͨ��name_2 �ж��ļ���vertex ����fragment��
		const char* m_name_3 = nullptr;	// ���ͣ����ڱ�ʶ�ض����ݣ�����ͨ��name_3 �ж�name_2==pos �������ʹ��light����cam����model
		void* p_data = nullptr;			// ����
		void* pp_data = nullptr;		// ���ݵĵ�ַ
		// void (*m_func)() = nullptr;	// ����
		std::function<void()> m_functional = NULL; // ������ʾ��ϸ���ݲ�������imgui��غ���
		ImU32 m_color = IM_COL32(134, 134, 134, 255);
		ImVec2 m_final_size = { 0,0 };	// �����ŵ�ʵ�ʴ�С
		ImVec2 m_size = { 0,0 };	// Slot��Node��ռ�õĴ�С
		ImVec2 m_pos = { 0,0 };		// Slot��Node�е�λ��ƫ��
		ImVec2 m_pos_next = { 0,0 };// ��һ��Slot��Node�е�λ��ƫ��
        ImVec2 m_point_pos = {0,0};         // Point start
		bool m_input = false;		// Ĭ��Input

		void setName(const std::string& _name);
		void setName2(const std::string& _name);
		void setName3(const std::string& _name);
	};
	struct ENGINE_API_THIRDPARTY SlotTemplateInput :public SlotTemplate {
		SlotTemplateInput() { m_input = true; }
	};
	struct ENGINE_API_THIRDPARTY SlotTemplateOutput :public SlotTemplate {
		SlotTemplateOutput() { m_input = false; }
	};


	// �����ʽģ��
	struct ENGINE_API_THIRDPARTY Template {
		static Template* newTemplate();
		static void deleteTemplate(Template*& _temp) ;
		virtual ~Template();
		Template() = default;
		Template(uint32_t _input_count, uint32_t _output_count);
		ImU32 m_header_color		= IM_COL32(160, 160, 180, 255);
		ImU32 m_bg_color			= IM_COL32(100, 100, 140, 255);
		ImU32 m_bg_color_over		= IM_COL32(110, 110, 150, 255);
		std::vector<SlotTemplate*> p_inputs;
		int m_original_input_count = 0;
		std::vector<SlotTemplate*> p_outputs;
		int m_original_output_count = 0;

		void addSlotTemplate(SlotTemplate* _temp, bool _input);
		void addSlotTemplates(const std::vector<SlotTemplate*>& _temps_in, bool _input);
		void addSlotTemplates(const std::vector<SlotTemplate*>& _temps_in, const std::vector<SlotTemplate*>& _temps_out);
		void addInputSlotTemplate(SlotTemplateInput* _temp);
		void addInputSlotTemplates(const std::vector<SlotTemplateInput*>& _temps);
		void addOutputSlotTemplate(SlotTemplateOutput* _temp);
		void addOutputSlotTemplates(const std::vector<SlotTemplateOutput*>& _temps);
		void setInputName(uint32_t _index, const std::string& _name);
		void setInputColor(uint32_t _index, const ImColor& _col);
		void setOutputName(uint32_t _index, const std::string& _name);
		void setOutputColor(uint32_t _index, const ImColor& _col);
	};

	// ��ʾ�ýڵ�, ����������
	struct ENGINE_API_THIRDPARTY Node {
		static Node* createNode(const std::string& _name,TemplateIndex _index);
		static void deleteNode(Node*& _node);
		Node(const std::string& _name,TemplateIndex _index);
		virtual ~Node();
		const char* m_name = nullptr;
		TemplateIndex m_template_index = 0;		// ģ������
		ImVec2 m_rect = { 0,0 };
		ImVec2 m_pos = { 0,0 };
		ImVec2 m_start = { 0,0 };				// Node��ʼλ��
		ImVec2 m_slot_start_pos = { 0,0 };		// Slot��ʼλ��
		bool m_select = false;
	};

	// ��������
	struct ENGINE_API_THIRDPARTY Link
	{
		NodeIndex m_input_node_index;		// ����ڵ�����
		SlotIndex m_input_slot_index;		// ������
		NodeIndex m_output_node_index;		// ����ڵ�����
		SlotIndex m_output_slot_index;		// ������
	};

	// �ڵ�ӿ�
	// ʵ�ָýӿڣ��Դ���beginNode���л���
	struct Delegate {
		inline static Delegate* g_delegate = nullptr;
		// ����
		virtual bool allowedLink(NodeIndex from, NodeIndex to) = 0;

		// ѡ��ڵ�
		virtual void selectNode(NodeIndex nodeIndex, bool selected) = 0;
		// �ƶ��ڵ�
		virtual void moveSelectedNodes(const ImVec2 delta) = 0;

		// �������
		virtual void addLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex) = 0;
		// ɾ������
		virtual void delLink(LinkIndex linkIndex) = 0;

		// ��ȡģ������
		virtual const size_t getTemplateCount() = 0;
		// ��ȡģ��
		virtual Template* getTemplate(TemplateIndex index) = 0;

		// ��ȡ�ڵ�����
		virtual const size_t getNodeCount() = 0;
		// ��ȡ�ڵ�
		virtual Node* getNode(NodeIndex index) = 0;

		// ��ȡ��������
		virtual const size_t getLinkCount() = 0;
		// ��ȡ����
		virtual Link* getLink(LinkIndex index) = 0;

		virtual ~Delegate() = default;
	};

	// ����hover�ж�
	ENGINE_API_THIRDPARTY bool beginCanvas(ViewState& _view_state, Options& _options);
	ENGINE_API_THIRDPARTY void endCanvas();

	// ��������
	ENGINE_API_THIRDPARTY void drawGrid(const ViewState& _view_state, const Options& _options,const ImVec2& _wnd_pos,const ImVec2& _canvas_size);

	// ����ָ���ڵ�, �����Ƿ�Hover/ Actice
	ENGINE_API_THIRDPARTY bool beginNode(ViewState& _view_state, Options& _options,Delegate& _delegate,uint32_t _node_index);
	ENGINE_API_THIRDPARTY void endNode();

	// Slot
	ENGINE_API_THIRDPARTY bool beginSlotTemplate(Node* _node, uint32_t _node_index, Template* _temp, std::vector<SlotTemplate*> _slots, uint32_t _index);
	ENGINE_API_THIRDPARTY void endSlotTemplate();

	// ����ѡȡ
	void handleQuadSelection();
	// ��������
	bool handleConnections();
	// ��������ͼ, ��������Ƿ���MiniMap
	bool drawMiniMap();
	// ��������
	void drawLink();
    // �Ƿ�ָ��, ���� = _start , ���� = _start + _size;
    bool isRectHover(const ImVec2& _start,const ImVec2& _size);
}

