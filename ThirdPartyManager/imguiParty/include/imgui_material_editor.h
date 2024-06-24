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
		ImVec2 m_slot_size = { 200.0f,25.0f };						// 插槽大小
		ImVec2 m_slot_name_padding = { 30,0 };						// Slot 名称偏移
		ImVec2 m_canvas_start_pos = { 0,0 };						// 画布起始位置
		ImU32 m_bg_color = IM_COL32(40, 40, 40, 255);				// 画布背景
		ImU32 m_grid_color_0 = IM_COL32(0, 0, 0, 60);				// 细线颜色
		ImU32 m_grid_color_1 = IM_COL32(0, 0, 0, 160);				// 粗线颜色
		ImU32 m_text_color = IM_COL32(255, 255, 255, 255);			// 文字颜色
		ImU32 m_select_border_color = IM_COL32(255, 130, 30, 255);	// 选中边框颜色
		ImU32 m_quad_selection = IM_COL32(255, 32, 32, 64);			// 选区颜色
		ImU32 m_quad_selection_border = IM_COL32(255, 32, 32, 64);	// 选区边框颜色
		float m_point_size = 8.0f;									// Point 半径
		float m_point_active_size = 16.0f;							// Point Hover半径
		float m_grid_size = 64.0f;									// 网格大小
		float m_zoom_ratio = 0.1f;									// 滚轮缩放因子
		float m_zoom_lerp_factor = 0.25f;							// 缩放因子
		float m_min_zoom = 0.2f, m_max_zoom = 2.3f;					// 缩放范围
		float m_border_thickness = 6.f;								// 边框粗细
		float m_select_border_thickness = 6.f;						// 边框选中粗细
		float m_rouding = 3.f;										// 圆角
	};

    struct DebugInfo {
        int m_select_node_index = -1;
        int m_hover_node_index = -1;
        int m_hover_slot_index = -1;
    };
	struct ENGINE_API_THIRDPARTY ViewState {
        ImVec2 m_mouse_position = {0,0};
        ImVec2 m_mouse_position_in_canvas = {0,0};
		ImVec2 m_window_position = {0,0};		// 窗口位置
		ImVec2 m_canvas_size = { 0,0 };			// 画布大小
		ImVec2 m_position = { 0,0 };			// 摄像位置
		ImVec2 m_cursor_screen_pos = { 0,0 };	// 下一输出位置
		ImVec2 m_current_slot_start_pos = { 0,0 };// 当前Slot起始位置，用于计算Slot高度
		ImVec2 m_imgui_start = { 0,0 };			// 当前imgui控件起始位置
		float m_factor = 1.0f;					// 当前zoom factor
		float m_factor_target = 1.0f;			// 目标zoom factor, 使用Options::m_zoom_lerp_factor插值
		bool m_is_hovered = false;				// 是否hover
		bool m_is_active = false;				// 是否按下鼠标
        DebugInfo m_debug_info;
	};

	// Slot 样式
	struct ENGINE_API_THIRDPARTY SlotTemplate {
		const char* m_name = nullptr;
		const char* m_name_2 = nullptr;	// 副名称，用于标识特定内容，例如通过name_2 判断文件是vertex 还是fragment等
		const char* m_name_3 = nullptr;	// 类型，用于标识特定内容，例如通过name_3 判断name_2==pos 的情况下使用light还是cam还是model
		void* p_data = nullptr;			// 数据
		void* pp_data = nullptr;		// 数据的地址
		// void (*m_func)() = nullptr;	// 弃用
		std::function<void()> m_functional = NULL; // 用于显示详细数据并操作的imgui相关函数
		ImU32 m_color = IM_COL32(134, 134, 134, 255);
		ImVec2 m_final_size = { 0,0 };	// 带缩放的实际大小
		ImVec2 m_size = { 0,0 };	// Slot在Node中占用的大小
		ImVec2 m_pos = { 0,0 };		// Slot在Node中的位置偏移
		ImVec2 m_pos_next = { 0,0 };// 下一个Slot在Node中的位置偏移
        ImVec2 m_point_pos = {0,0};         // Point start
		bool m_input = false;		// 默认Input

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


	// 输出样式模板
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

	// 显示用节点, 不包含数据
	struct ENGINE_API_THIRDPARTY Node {
		static Node* createNode(const std::string& _name,TemplateIndex _index);
		static void deleteNode(Node*& _node);
		Node(const std::string& _name,TemplateIndex _index);
		virtual ~Node();
		const char* m_name = nullptr;
		TemplateIndex m_template_index = 0;		// 模板索引
		ImVec2 m_rect = { 0,0 };
		ImVec2 m_pos = { 0,0 };
		ImVec2 m_start = { 0,0 };				// Node起始位置
		ImVec2 m_slot_start_pos = { 0,0 };		// Slot起始位置
		bool m_select = false;
	};

	// 连接属性
	struct ENGINE_API_THIRDPARTY Link
	{
		NodeIndex m_input_node_index;		// 输入节点索引
		SlotIndex m_input_slot_index;		// 输入插槽
		NodeIndex m_output_node_index;		// 输出节点索引
		SlotIndex m_output_slot_index;		// 输出插槽
	};

	// 节点接口
	// 实现该接口，以传入beginNode进行绘制
	struct Delegate {
		inline static Delegate* g_delegate = nullptr;
		// 链接
		virtual bool allowedLink(NodeIndex from, NodeIndex to) = 0;

		// 选择节点
		virtual void selectNode(NodeIndex nodeIndex, bool selected) = 0;
		// 移动节点
		virtual void moveSelectedNodes(const ImVec2 delta) = 0;

		// 添加链接
		virtual void addLink(NodeIndex inputNodeIndex, SlotIndex inputSlotIndex, NodeIndex outputNodeIndex, SlotIndex outputSlotIndex) = 0;
		// 删除链接
		virtual void delLink(LinkIndex linkIndex) = 0;

		// 获取模板数量
		virtual const size_t getTemplateCount() = 0;
		// 获取模板
		virtual Template* getTemplate(TemplateIndex index) = 0;

		// 获取节点数量
		virtual const size_t getNodeCount() = 0;
		// 获取节点
		virtual Node* getNode(NodeIndex index) = 0;

		// 获取连接数量
		virtual const size_t getLinkCount() = 0;
		// 获取连接
		virtual Link* getLink(LinkIndex index) = 0;

		virtual ~Delegate() = default;
	};

	// 包含hover判定
	ENGINE_API_THIRDPARTY bool beginCanvas(ViewState& _view_state, Options& _options);
	ENGINE_API_THIRDPARTY void endCanvas();

	// 绘制网格
	ENGINE_API_THIRDPARTY void drawGrid(const ViewState& _view_state, const Options& _options,const ImVec2& _wnd_pos,const ImVec2& _canvas_size);

	// 绘制指定节点, 返回是否Hover/ Actice
	ENGINE_API_THIRDPARTY bool beginNode(ViewState& _view_state, Options& _options,Delegate& _delegate,uint32_t _node_index);
	ENGINE_API_THIRDPARTY void endNode();

	// Slot
	ENGINE_API_THIRDPARTY bool beginSlotTemplate(Node* _node, uint32_t _node_index, Template* _temp, std::vector<SlotTemplate*> _slots, uint32_t _index);
	ENGINE_API_THIRDPARTY void endSlotTemplate();

	// 处理选取
	void handleQuadSelection();
	// 处理连线
	bool handleConnections();
	// 绘制缩略图, 返回鼠标是否在MiniMap
	bool drawMiniMap();
	// 绘制连线
	void drawLink();
    // 是否被指向, 左上 = _start , 右下 = _start + _size;
    bool isRectHover(const ImVec2& _start,const ImVec2& _size);
}

