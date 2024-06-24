#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_material_editor.h"
#include <imgui/imgui_internal.h>
#include <unordered_map>
#ifndef _WIN32
#define strcpy_s(A,B,C) strcpy(A,C)
#endif


namespace ThirdParty::imgui {
    static Options *g_options = nullptr;
    static ViewState *g_view_state = nullptr;
    static Delegate *g_delegate = nullptr;
    static NodeOperation g_node_operation = NO_None;
    static Node *g_node = nullptr;
    static ImDrawList* g_draw_list = nullptr;
    static uint32_t g_node_index = 0;
    static bool g_has_hover = false;
    static bool g_has_select = false;
    static int g_select_node_index = -1;         // 循环每个node 判断是否指向
    static int g_hover_node_index = -1;         // 循环每个node 判断是否指向
    static int g_hover_slot_index = -1;
    // TEMPLATE
    static ImVec2 editingNodeSource;
    static bool editingInput = false;
    static int g_editing_node_src_index = -1;    // editing src slot index
    static int g_editing_node_dst_index = -1;    // editing dst slot index
    static int g_editing_slot_src_index = -1;    // editing dst slot index
    static int g_editing_slot_dst_index = -1;    // editing dst slot index

    static inline float Distance(const ImVec2 &a, const ImVec2 &b) {
        return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    }

    ENGINE_API_THIRDPARTY void
    drawGrid(const ViewState &_view_state, const Options &_options, const ImVec2 &_wnd_pos, const ImVec2 &_canvas_size) {
        float gridSpace = _options.m_grid_size * _view_state.m_factor;
        int divx = static_cast<int>(-_view_state.m_position.x / _options.m_grid_size);
        int divy = static_cast<int>(-_view_state.m_position.y / _options.m_grid_size);
        for (float x = fmodf(_view_state.m_position.x * _view_state.m_factor, gridSpace);
             x < _canvas_size.x; x += gridSpace, divx++) {
            bool tenth = !(divx % 10);
            g_draw_list->AddLine(ImVec2(x, 0.0f) + _wnd_pos, ImVec2(x, _canvas_size.y) + _wnd_pos,
                                tenth ? _options.m_grid_color_1 : _options.m_grid_color_0);
        }
        for (float y = fmodf(_view_state.m_position.y * _view_state.m_factor, gridSpace);
             y < _canvas_size.y; y += gridSpace, divy++) {
            bool tenth = !(divy % 10);
            g_draw_list->AddLine(ImVec2(0.0f, y) + _wnd_pos, ImVec2(_canvas_size.x, y) + _wnd_pos,
                                tenth ? _options.m_grid_color_1 : _options.m_grid_color_0);
        }
    }

    ENGINE_API_THIRDPARTY bool
    beginNode(ViewState &_view_state, Options &_options, Delegate &_delegate, uint32_t _node_index) {
        g_delegate = &_delegate;
        g_has_hover = _node_index != 0 && g_has_hover;
        g_has_select = _node_index != 0 && g_has_select;
        const bool old_any_active = ImGui::IsAnyItemActive();
        const auto io = ImGui::GetIO();
        bool _rel = false;
        auto _node = _delegate.getNode(_node_index);
        auto _template = _delegate.getTemplate(_node->m_template_index);
        auto _node_size = _node->m_rect * _view_state.m_factor;
        auto _select = _node->m_select;
        auto _color =
                g_hover_node_index == _node_index && !_select ? _template->m_bg_color_over : _template->m_bg_color;
        auto _color_select = g_options->m_select_border_color;
        auto _rouding = g_options->m_rouding;
        ImGui::PushID(_node);
        // 绘制整体
        ImVec2 _min_rect =
                _view_state.m_window_position + (_view_state.m_position + _node->m_pos) * _view_state.m_factor;
        ImVec2 _max_rect = _min_rect + _node_size;
        g_draw_list->ChannelsSetCurrent(1); // Background
        g_draw_list->AddRect(
                _min_rect,
                _max_rect,
                _select ? _color_select : _color,
                _rouding,
                ImDrawFlags_RoundCornersAll,
                _select ? g_options->m_select_border_thickness : g_options->m_border_thickness);
        g_draw_list->AddRectFilled(_min_rect, _max_rect, _color, _rouding);
        ImRect viewPort(_view_state.m_window_position, _view_state.m_window_position + _view_state.m_canvas_size);
        float maxHeight = ImMin(viewPort.Max.y, _max_rect.y) - _min_rect.y;
        float maxWidth = ImMin(viewPort.Max.x, _max_rect.x) - _min_rect.x;
        if (maxHeight > 0.0f && maxWidth > 0.0f) {
            ImGui::SetCursorScreenPos(_min_rect);
            ImGui::InvisibleButton("node", ImVec2{maxWidth, maxHeight});
            _rel = ImGui::IsItemHovered() || ImGui::IsItemActive();
            if(ImGui::IsItemHovered() || isRectHover(_min_rect,_node_size)) {
                g_has_hover = true;
                g_hover_node_index = _node_index;
            } else {
                if(_node_index == g_delegate->getNodeCount() -1 && !g_has_hover)
                    g_hover_node_index = -1;
            }
        }

        // 绘制头部
        auto _slot_size = g_options->m_slot_size * g_view_state->m_factor;
        _max_rect = _min_rect + _slot_size;
        g_draw_list->AddRect(
                _min_rect,
                _max_rect,
                _template->m_header_color,
                _rouding,
                ImDrawFlags_RoundCornersAll);
        g_draw_list->AddRectFilled(
                _min_rect,
                _max_rect,
                _template->m_header_color,
                _rouding,
                ImDrawFlags_RoundCornersAll);

        ImGui::SetWindowFontScale(g_view_state->m_factor);
        g_draw_list->PushClipRect(_min_rect, _max_rect, true);
        g_draw_list->AddText(_min_rect, g_options->m_text_color, _node->m_name);
        g_draw_list->PopClipRect();

        // 设置Slot起始位置
        _node->m_slot_start_pos = {_min_rect.x, _max_rect.y};
        // 保存起始位置
        _node->m_start = _min_rect;

        g_node = _node;
        g_node_index = _node_index;

        // 控制
        bool node_widget_active = (!old_any_active && ImGui::IsAnyItemActive());
        bool node_moving_active = ImGui::IsItemActive();
        bool node_hover = ImGui::IsItemHovered();

        // 选中Node
        if (ImGui::IsWindowFocused()) {
            if (node_widget_active || node_moving_active) {
                if (!_node->m_select) {
                    if (!io.KeyShift) {
                        const auto nodeCount = _delegate.getNodeCount();
                        for (size_t i = 0; i < nodeCount; i++) {
                            _delegate.selectNode(i, false);
                        }
                        g_select_node_index = -1;
                    }
                    _delegate.selectNode(_node_index, true);
                    g_select_node_index = _node_index;
                }
            }
        }

        if (node_moving_active && io.MouseDown[0] && node_hover) {
            if (g_node_operation == NO_None) {
                g_node_operation = NO_MovingNodes;
            }
        }
        return _rel;
    }

    ENGINE_API_THIRDPARTY void endNode() {
        ImGui::SetWindowFontScale(1.0f);
        // 计算实际高度
        auto _node_y = ImGui::GetCursorScreenPos().y;
        auto Y = (_node_y - g_node->m_start.y) / g_view_state->m_factor;
        if(g_delegate) {
            auto _temp = g_delegate->getTemplate(g_node->m_template_index);
            if (_temp->p_outputs.empty() && _temp->p_inputs.empty())
                g_node->m_rect.y = g_options->m_slot_size.y;
            else
                g_node->m_rect.y = Y;
        }
        ImGui::PopID();
    }

    void SlotTemplate::setName(const std::string &_name) {
        char *_p_name = new char[_name.length() + 1];
        strcpy_s(_p_name, sizeof(char) * _name.length() + 1, _name.c_str());
        this->m_name = _p_name;
    }

	void SlotTemplate::setName2(const std::string& _name)
	{
		char* _p_name = new char[_name.length() + 1];
		strcpy_s(_p_name, sizeof(char) * _name.length() + 1, _name.c_str());
		this->m_name_2 = _p_name;
	}

	void SlotTemplate::setName3(const std::string& _name)
	{
		char* _p_name = new char[_name.length() + 1];
		strcpy_s(_p_name, sizeof(char) * _name.length() + 1, _name.c_str());
		this->m_name_3 = _p_name;
	}

    Template * Template::newTemplate()  {return new Template;}

    void Template::deleteTemplate(Template *&_temp){delete _temp;_temp = nullptr;}

    Template::~Template() {
        for (const auto &_input: p_inputs) {
            delete[] _input->m_name;
            if (_input->m_name_2)
                delete[] _input->m_name_2;
			if (_input->m_name_3)
				delete[] _input->m_name_3;
            delete _input;
        }
        p_inputs.clear();

        for (const auto &_output: p_outputs) {
            delete[] _output->m_name;
            if (_output->m_name_2)
                delete[] _output->m_name_2;
			if (_output->m_name_3)
				delete[] _output->m_name_3;
            delete _output;
        }
        p_outputs.clear();
    }

    void Template::setInputName(uint32_t _index, const std::string &_name) {
        char *_p_name = new char[_name.length() + 1];
        strcpy_s(_p_name, sizeof(char) * _name.length() + 1, _name.c_str());
        p_inputs[_index]->m_name = _p_name;
    }

    void Template::setInputColor(uint32_t _index, const ImColor &_col) {
        p_inputs[_index]->m_color = _col;
    }

    void Template::setOutputName(uint32_t _index, const std::string &_name) {
        char *_p_name = new char[_name.length() + 1];
        strcpy_s(_p_name, sizeof(char) * _name.length() + 1, _name.c_str());
        p_outputs[_index]->m_name = _p_name;
    }

    void Template::setOutputColor(uint32_t _index, const ImColor &_col) {
        p_outputs[_index]->m_color = _col;
    }

    Node * Node::createNode(const std::string &_name, TemplateIndex _index) {
        return new Node(_name,_index);
    }

    void Node::deleteNode(Node *&_node) {
        delete _node;
        _node = nullptr;
    }

    Node::Node(const std::string &_name, TemplateIndex _index):m_template_index(_index) {
        char *_p_name = new char[_name.length() + 1];
        strcpy_s(_p_name, sizeof(char) * _name.length() + 1, _name.c_str());
        m_name = _p_name;
        m_rect = {200,50};
    }

    Node::~Node() {
        delete[] m_name;
    }

    Template::Template(uint32_t _input_count, uint32_t _output_count) {
        p_inputs.resize(_input_count);
        for (auto &_input: p_inputs) _input = new SlotTemplateInput;
        p_outputs.resize(_output_count);
        for (auto &_output: p_outputs) _output = new SlotTemplateOutput;

        // 设定Node 背景色
        m_bg_color = IM_COL32(15, 15, 15, 255);
        m_bg_color_over = IM_COL32(30, 30, 30, 255);
    }

    void Template::addSlotTemplate(SlotTemplate *_temp, bool _input) {
        auto& _p = _input? p_inputs:p_outputs;
        _p.push_back(_temp);
    }

    void Template::addSlotTemplates(const std::vector<SlotTemplate *> &_temps_in, bool _input) {
        auto& _p = _input? p_inputs:p_outputs;
        if(!_temps_in.empty())
            _p.insert(_p.end(),_temps_in.begin(),_temps_in.end());
    }

    void Template::addSlotTemplates(const std::vector<SlotTemplate *> &_temps_in,
        const std::vector<SlotTemplate *> &_temps_out) {
        addSlotTemplates(_temps_in,true);
        addSlotTemplates(_temps_out,false);
    }

    void Template::addInputSlotTemplate(SlotTemplateInput *_temp) {
        p_inputs.push_back(_temp);
    }

    void Template::addInputSlotTemplates(const std::vector<SlotTemplateInput *> &_temps) {
        auto& _p = p_inputs;
        if(!_temps.empty())
            _p.insert(_p.end(),_temps.begin(),_temps.end());
    }

    void Template::addOutputSlotTemplate(SlotTemplateOutput *_temp) {
        p_outputs.push_back(_temp);
    }

    void Template::addOutputSlotTemplates(const std::vector<SlotTemplateOutput *> &_temps) {
        auto& _p = p_outputs;
        if(!_temps.empty())
            _p.insert(_p.end(),_temps.begin(),_temps.end());
    }

    static SlotTemplate *g_slot_temp = nullptr;
    static uint32_t g_slot_index = 0;

    ENGINE_API_THIRDPARTY bool
    beginSlotTemplate(Node *_node, uint32_t _node_index, Template *_temp, std::vector<SlotTemplate *> _slots, uint32_t _index) {
        bool _rel = false;
        auto _scale = g_view_state->m_factor;
        auto _slot = _slots[_index];
        auto _slot_size = g_options->m_slot_size * _scale;
        auto _text_offset = g_options->m_slot_name_padding * _scale;
        bool _input = _slot->m_input;
        auto _point_size = g_options->m_point_size;
        auto _point_hover_size = _point_size * 1.4f;
        auto _text_size = ImGui::CalcTextSize(_slot->m_name);
        auto io = ImGui::GetIO();

        // Output Index 0
        if (!_input && _index == 0) _slot->m_pos = _node->m_slot_start_pos;
        else if (!_input) _slot->m_pos = _slots[_index - 1]->m_pos_next;
            // Input Index 0
        else if (_input && _index == 0 && !_temp->p_outputs.empty())
            _slot->m_pos = _temp->p_outputs[_temp->p_outputs.size() - 1]->m_pos_next;
        else if (_input && _index == 0 && _temp->p_outputs.empty()) _slot->m_pos = _node->m_slot_start_pos;
        else if (_input) _slot->m_pos = _slots[_index - 1]->m_pos_next;
        // 计算Slot区域大小
        auto _start = _slot->m_pos;
        auto _end = _start + _slot_size;
        auto _text_start = _input ? _start + ImVec2{_point_hover_size,0} :
                           ImVec2{_end.x - ImGui::CalcTextSize(_slot->m_name).x, _start.y} - ImVec2{_point_hover_size,0};

        ImGui::PushID(_slot);
        g_draw_list->ChannelsSetCurrent(2);
        // 显示
        g_draw_list->AddRect(_start, _end, _slot->m_color);
        g_draw_list->AddRectFilled(_start, _end, _slot->m_color);
        g_draw_list->PushClipRect(_start, _end, true);
        g_draw_list->AddText(_text_start, g_options->m_text_color, _slot->m_name);
        g_draw_list->PopClipRect();
        // 绘制Point
        g_draw_list->ChannelsSetCurrent(4);
        ImVec2 _point_start = _start + ImVec2{0, _point_size};
        if (!_input) _point_start.x = _end.x;
        if(isRectHover(_point_start-ImVec2{_point_hover_size / 2.f,_point_hover_size / 2.f},{_point_hover_size,_point_hover_size}))
        {
            _point_size *= 1.3f;
            auto _text_start_temp = (_input)? _point_start - ImVec2{_point_hover_size,0}:_point_start + ImVec2{_point_hover_size,0};
            // 显示Slot Name
            // 计算文字长度
            if(_input) {
                g_draw_list->PushClipRect(_text_start_temp - _text_size, _text_start_temp, true);
                g_draw_list->AddText(_text_start_temp - _text_size, g_options->m_text_color, _slot->m_name);
            }
            else {
                g_draw_list->PushClipRect(_text_start_temp, _text_start_temp + _text_size, true);
                g_draw_list->AddText(_text_start_temp, g_options->m_text_color, _slot->m_name);
            }
            g_draw_list->PopClipRect();

            // 如果鼠标按下，则进入绘制链接模式
            if(io.MouseDown[0] && g_node_operation != NO_EditingLink) {
                editingNodeSource = _point_start;       // 设置源节点
                editingInput = _input;                  // 是否是Input节点
                g_editing_node_src_index = _node_index; // src node index
                g_editing_slot_src_index = _index;      // src slot index
                g_node_operation = NO_EditingLink;
            }
            g_has_hover = true;
            g_hover_node_index = _node_index;
        }
        _slot->m_point_pos = _point_start;
        g_draw_list->AddCircleFilled(_point_start, _point_size, IM_COL32(0, 0, 0, 200));    // 边框
        g_draw_list->AddCircleFilled(_point_start, _point_size * 0.75f, _temp->m_header_color);

        // 设定控件输出Split
        g_draw_list->ChannelsSetCurrent(5);
        ImGui::SetCursorScreenPos({ _start.x, _end.y });
        if (_slot->m_functional != NULL) {
            ImGui::BeginChild(ImGui::GetID("Detail"), { _node->m_rect.x * _scale ,0 }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
            _slot->m_functional();
        }
        g_slot_temp = _slot;
        g_slot_index = _index;
        return true;
    }

    ENGINE_API_THIRDPARTY void endSlotTemplate() {
        if (g_slot_temp->m_functional != NULL) {
            ImGui::EndChild();
        }
        ImGui::PopID();
        auto _next_Y = ImGui::GetCursorScreenPos().y;
        auto _offset_Y = _next_Y - g_slot_temp->m_pos.y;
        // 计算下一slot输出位置
        g_slot_temp->m_pos_next = g_slot_temp->m_pos + ImVec2{0, _offset_Y};
        // 计算SlotSize
        g_slot_temp->m_size = {g_options->m_slot_size.x, _offset_Y};
        g_slot_temp->m_final_size = g_slot_temp->m_size * g_view_state->m_factor;

    }

    void handleQuadSelection() {
        if (!g_delegate) return;
        ImGuiIO &io = ImGui::GetIO();
        static ImVec2 quadSelectPos;
        auto nodeCount = g_delegate->getNodeCount();
        auto drawList = g_draw_list;
        auto options = *g_options;
        auto factor = g_view_state->m_factor;
        auto offset = g_view_state->m_window_position + g_view_state->m_position * factor;
        ImRect contentRect(g_view_state->m_window_position,
                           g_view_state->m_window_position + g_view_state->m_canvas_size);

        if (g_node_operation == NO_QuadSelecting && ImGui::IsWindowFocused()) {
            const ImVec2 bmin = ImMin(quadSelectPos, io.MousePos);
            const ImVec2 bmax = ImMax(quadSelectPos, io.MousePos);
            drawList->AddRectFilled(bmin, bmax, options.m_quad_selection_border, 1.f);
            drawList->AddRect(bmin, bmax, options.m_quad_selection, 1.f);
            if (!io.MouseDown[0]) {
                if (!io.KeyCtrl && !io.KeyShift) {
                    for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
                        g_delegate->selectNode(nodeIndex, false);
                    }
                    g_select_node_index = -1;
                }

                g_node_operation = NO_None;
                ImRect selectionRect(bmin, bmax);
                for (int nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
                    const auto node = g_delegate->getNode(nodeIndex);
                    ImVec2 nodeRectangleMin = offset + node->m_pos * factor;
                    ImVec2 nodeRectangleMax = nodeRectangleMin + node->m_rect * factor;
                    if (selectionRect.Overlaps(ImRect(nodeRectangleMin, nodeRectangleMax))) {
                        if (io.KeyCtrl) {
                            g_delegate->selectNode(nodeIndex, false);
                        } else {
                            g_delegate->selectNode(nodeIndex, true);
                        }
                    } else {
                        if (!io.KeyShift) {
                            g_delegate->selectNode(nodeIndex, false);
                        }
                    }
                }
            }
        } else if (g_node_operation == NO_None && io.MouseDown[0] && ImGui::IsWindowFocused() &&
                   contentRect.Contains(io.MousePos)) {
            g_node_operation = NO_QuadSelecting;
            quadSelectPos = io.MousePos;
        }
    }

    bool drawMiniMap() {
        if (!g_delegate) return false;
        auto options = g_options;
        auto delegate = g_delegate;
        auto drawList =g_draw_list;
        ImRect mMinimap = {0.75f, 0.8f, 0.99f, 0.99f};
        if (Distance(mMinimap.Min, mMinimap.Max) <= FLT_EPSILON) {
            return false;
        }

        const size_t nodeCount = delegate->getNodeCount();

        if (!nodeCount) {
            return false;
        }

        ImVec2 min(FLT_MAX, FLT_MAX);
        ImVec2 max(-FLT_MAX, -FLT_MAX);
        const ImVec2 margin(50, 50);
        for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
            auto node = delegate->getNode(nodeIndex);
            ImRect rect = {node->m_pos, node->m_pos + node->m_rect};
            min = ImMin(min, rect.Min - margin);
            min = ImMin(min, rect.Max + margin);
            max = ImMax(max, rect.Min - margin);
            max = ImMax(max, rect.Max + margin);
        }

        // add view in world space
        auto canvasSize = g_view_state->m_canvas_size;
        auto scale = g_view_state->m_factor;
        auto canvasPos = g_view_state->m_position;
        auto windowPos = g_view_state->m_window_position;
        const ImVec2 worldSizeView = canvasSize / scale;
        const ImVec2 viewMin(-canvasPos.x, -canvasPos.y);
        const ImVec2 viewMax = viewMin + worldSizeView;
        min = ImMin(min, viewMin);
        max = ImMax(max, viewMax);
        const ImVec2 nodesSize = max - min;
        const ImVec2 middleWorld = (min + max) * 0.5f;
        const ImVec2 minScreen = windowPos + mMinimap.Min * canvasSize;
        const ImVec2 maxScreen = windowPos + mMinimap.Max * canvasSize;
        const ImVec2 viewSize = maxScreen - minScreen;
        const ImVec2 middleScreen = (minScreen + maxScreen) * 0.5f;
        const float ratioY = viewSize.y / nodesSize.y;
        const float ratioX = viewSize.x / nodesSize.x;
        const float factor = ImMin(ImMin(ratioY, ratioX), 1.f);

        drawList->AddRectFilled(minScreen, maxScreen, IM_COL32(30, 30, 30, 200), 3, ImDrawFlags_RoundCornersAll);

        for (NodeIndex nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
            auto node = delegate->getNode(nodeIndex);
            const auto nodeTemplate = delegate->getTemplate(node->m_template_index);

            ImRect rect = {node->m_pos, node->m_pos + node->m_rect};
            rect.Min -= middleWorld;
            rect.Min *= factor;
            rect.Min += middleScreen;

            rect.Max -= middleWorld;
            rect.Max *= factor;
            rect.Max += middleScreen;

            drawList->AddRectFilled(rect.Min, rect.Max, nodeTemplate->m_bg_color, 1, ImDrawFlags_RoundCornersAll);
            if (node->m_select) {
                drawList->AddRect(rect.Min, rect.Max, options->m_select_border_color, 1, ImDrawFlags_RoundCornersAll);
            }
        }

        // add view
        ImVec2 viewMinScreen = (viewMin - middleWorld) * factor + middleScreen;
        ImVec2 viewMaxScreen = (viewMax - middleWorld) * factor + middleScreen;
        drawList->AddRectFilled(viewMinScreen, viewMaxScreen, IM_COL32(255, 255, 255, 32), 1,
                                ImDrawFlags_RoundCornersAll);
        drawList->AddRect(viewMinScreen, viewMaxScreen, IM_COL32(255, 255, 255, 128), 1, ImDrawFlags_RoundCornersAll);

        ImGuiIO &io = ImGui::GetIO();
        const bool mouseInMinimap = ImRect(minScreen, maxScreen).Contains(io.MousePos);
        if (mouseInMinimap && io.MouseClicked[0]) {
            const ImVec2 clickedRatio = (io.MousePos - minScreen) / viewSize;
            const ImVec2 worldPosCenter = ImVec2(ImLerp(min.x, max.x, clickedRatio.x),
                                                 ImLerp(min.y, max.y, clickedRatio.y));

            ImVec2 worldPosViewMin = worldPosCenter - worldSizeView * 0.5;
            ImVec2 worldPosViewMax = worldPosCenter + worldSizeView * 0.5;
            if (worldPosViewMin.x < min.x) {
                worldPosViewMin.x = min.x;
                worldPosViewMax.x = worldPosViewMin.x + worldSizeView.x;
            }
            if (worldPosViewMin.y < min.y) {
                worldPosViewMin.y = min.y;
                worldPosViewMax.y = worldPosViewMin.y + worldSizeView.y;
            }
            if (worldPosViewMax.x > max.x) {
                worldPosViewMax.x = max.x;
                worldPosViewMin.x = worldPosViewMax.x - worldSizeView.x;
            }
            if (worldPosViewMax.y > max.y) {
                worldPosViewMax.y = max.y;
                worldPosViewMin.y = worldPosViewMax.y - worldSizeView.y;
            }
            g_view_state->m_position = ImVec2(-worldPosViewMin.x, -worldPosViewMin.y);
        }
        return mouseInMinimap;
    }

    void drawLink() {
        if (!g_delegate) return;
        auto _link_count = g_delegate->getLinkCount();
        auto _scale = g_view_state->m_factor;
        for (LinkIndex linkIndex = 0; linkIndex < _link_count; linkIndex++) {
            auto _link = g_delegate->getLink(linkIndex);
            // dst node
            auto _node = g_delegate->getNode(_link->m_input_node_index);
            auto _template = g_delegate->getTemplate(_node->m_template_index);
            auto _slot = _template->p_inputs[_link->m_input_slot_index];
            // p1
            auto _p1 = _slot->m_point_pos;

            // src node
            auto _node2 = g_delegate->getNode(_link->m_output_node_index);
            auto _template2 = g_delegate->getTemplate(_node2->m_template_index);
            auto _slot2 = _template2->p_outputs[_link->m_output_slot_index];
            // p2
            auto _p2 = _slot2->m_point_pos;

            // Curves
            bool highlightCons = g_hover_node_index == _link->m_input_node_index || g_hover_node_index == _link->m_output_node_index;
            auto _color = _template->m_header_color | (highlightCons?0xF0F0F0 : 0);
            g_draw_list->AddBezierCubic(_p1,_p1 - ImVec2(50, 0) * _scale, _p2 - ImVec2(-50, 0) * _scale,_p2,0xFF000000,g_options->m_border_thickness);
            g_draw_list->AddBezierCubic(_p1,_p1 - ImVec2(50, 0) * _scale, _p2 - ImVec2(-50, 0) * _scale,_p2,_color,g_options->m_border_thickness);
        }
    }

    bool handleConnections() {
        if (!g_delegate) return false;
        auto io = ImGui::GetIO();
        auto _mouse_pos = io.MousePos;
        auto _point_hover_size = g_options->m_point_size * 1.3f;
        auto _scale = g_view_state->m_factor;
        auto _link_count = g_delegate->getLinkCount();

        float closestDistance = FLT_MAX;
        SlotIndex closestConn = -1;
        ImVec2 closestPos;
        // 连线模式中...
        if (g_node_operation == NO_EditingLink) {
            // 寻找Node中最近可连接Slot点
            if(g_hover_node_index >= 0) {
                g_hover_slot_index = -1;
                auto _node = g_delegate->getNode(g_hover_node_index);
                auto _temp = g_delegate->getTemplate(_node->m_template_index);
                // input 对 output. output 对 input
                auto _slots = editingInput?_temp->p_outputs:_temp->p_inputs;
                auto _slot_count = _slots.size();
                for (size_t i = 0; i < _slot_count; i++) {
                    auto _slot = _slots[i];
                    ImVec2 p = _slot->m_point_pos;
                    float distance = Distance(p,_mouse_pos);
                    bool overCon = (g_node_operation == NO_None || g_node_operation == NO_EditingLink) &&
                                   (distance < g_options->m_point_size * 2.f) && (distance < closestDistance);
                    ImRect _slot_rect = {_slot->m_pos,_slot->m_pos + _slot->m_size * _scale};
                    bool _slot_hover = _slot_rect.Contains(_mouse_pos);
                    if(_slot_hover) g_hover_slot_index = i;
                    if ((overCon || (_slot_hover && closestConn == -1 && (editingInput == !_slot->m_input) && g_node_operation == NO_EditingLink)))
                    {
                        closestDistance = distance;
                        closestConn = i;
                        closestPos = p;
                    }
                }
                // 最近的slotIndex
                if(closestConn != -1) {
                    g_editing_slot_dst_index = closestConn;
                    g_editing_node_dst_index = g_hover_node_index;
                    auto _point_hover_start = closestPos;
                    g_draw_list->AddCircleFilled(_point_hover_start, _point_hover_size * 0.75f, IM_COL32(0, 0, 0, 200));
                    g_draw_list->AddCircleFilled(_point_hover_start, _point_hover_size, IM_COL32(255, 255, 255, 255));
                }

            }

            // 连线模式结束[鼠标松开], 进行连接
            if (!io.MouseDown[0]) {
                if (g_editing_slot_src_index >= 0 && g_editing_slot_dst_index >= 0) {
                    Link n1;
                    if(editingInput) {
                        n1 = Link{
                                g_editing_node_src_index,
                                g_editing_slot_src_index,
                                g_editing_node_dst_index,
                                g_editing_slot_dst_index};
                    } else {
                        n1 = Link{
                            g_editing_node_dst_index,
                            g_editing_slot_dst_index,
                            g_editing_node_src_index,
                            g_editing_slot_src_index};
                    }
                    bool _is_exist = false;
                    for(uint32_t i=0;i<_link_count;i++) {
                        const auto& _link = g_delegate->getLink(i);
                        if(!memcmp(&_link,&n1,sizeof(Link))) {
                            _is_exist = true;
                            break;
                        }
                    }
                    // 创建新链接
                    if (!_is_exist) {
                        // 删除旧链接
                        for (uint32_t i = 0; i < _link_count; i++) {
                            const auto &_link = g_delegate->getLink(i);
                            if (_link->m_input_node_index == n1.m_input_node_index &&
                                _link->m_input_slot_index == n1.m_input_slot_index) {
                                g_delegate->delLink(i);
                                break;
                            }
                        }
                        g_delegate->addLink(n1.m_input_node_index,n1.m_input_slot_index,n1.m_output_node_index,n1.m_output_slot_index);
                    }
                }
                g_editing_node_dst_index = -1;
                g_editing_slot_dst_index = -1;
                g_node_operation = NO_None;
            }
        }

        // 单击Input Slot取消
        if(g_node_operation == NO_None && !io.MouseDown[0] && g_editing_node_src_index >=0 && g_editing_slot_src_index >= 0) {
            g_node_operation = NO_EditingLink;
            auto _node = g_delegate->getNode(g_editing_node_src_index);
            auto _temp = g_delegate->getTemplate(_node->m_template_index);
            auto _slot = editingInput? _temp->p_inputs[g_editing_slot_src_index] : _temp->p_outputs[g_editing_slot_src_index];
            editingInput = _slot->m_input;
            editingNodeSource = closestPos;
            g_editing_node_src_index = g_hover_node_index;
            g_editing_slot_src_index = closestConn;
            // 只重置input
            if (editingInput)
            {
                // remove existing link
                for (int linkIndex = 0; linkIndex < _link_count; linkIndex++)
                {
                    const auto link = g_delegate->getLink(linkIndex);
                    if (link->m_input_node_index == g_editing_node_src_index)
                    {
                        g_delegate->delLink(linkIndex);
                        break;
                    }
                }
            }
            g_editing_node_src_index = -1;
            g_editing_slot_src_index = -1;
        }

        return true;
    }

    ENGINE_API_THIRDPARTY bool beginCanvas(ViewState &_view_state, Options &_options) {
        g_options = &_options;
        g_view_state = &_view_state;
        const ImVec2 windowPos = ImGui::GetCursorScreenPos();
        const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        const ImVec2 scrollRegionLocalPos(0, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
        auto _rel = ImGui::BeginChild("Canvas", {0, 0}, ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar);
        if (_rel) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, _options.m_bg_color);         // Set a background color
            auto _wnd_pos = _options.m_canvas_start_pos;
            _view_state.m_window_position = _wnd_pos;
            _view_state.m_mouse_position_in_canvas = _view_state.m_mouse_position - _wnd_pos;
            auto _canvas_size = ImGui::GetWindowSize();
            _view_state.m_canvas_size = _canvas_size;
            if (_canvas_size.x < 50.0f) _canvas_size.x = 50.0f;
            if (_canvas_size.y < 50.0f) _canvas_size.y = 50.0f;
            // Draw border and background color
            ImGuiIO &io = ImGui::GetIO();
            g_draw_list = ImGui::GetWindowDrawList();
            ImGui::BeginChildFrame(2024423,canvasSize);
            ImGui::SetCursorPos(windowPos);
            ImGui::BeginGroup();
            _view_state.m_is_hovered = isRectHover(_wnd_pos,_canvas_size);
            g_draw_list->PushClipRect(_wnd_pos,_canvas_size +_wnd_pos,true);
            g_draw_list->AddRectFilled(_wnd_pos, _canvas_size + _wnd_pos, _options.m_bg_color);
            drawGrid(_view_state, _options, _wnd_pos, _canvas_size);
//            ImGui::SetCursorScreenPos(_wnd_pos);
            //ImGui::InvisibleButton("canvas", _canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
            if ((ImGui::IsMouseDown(0) || ImGui::IsMouseDown(1) || ImGui::IsMouseDown(2)) && _view_state.m_is_hovered)
                _view_state.m_is_active = true;
            else
                _view_state.m_is_active = false;

            // Display links
            // 5: MiniMap
            // 4: Point & Link
            // 3: ImGui // 似乎对ImGui控件无效
            // 2: Template
            // 1: Node
            // 0: Grid
            g_draw_list->ChannelsSplit(10);

            // 绘制缩略图
            g_draw_list->ChannelsSetCurrent(5); // minimap
            const bool inMinimap = drawMiniMap();

            // Focus rectangle
            if (ImGui::IsWindowFocused()) {
                g_draw_list->AddRect(_wnd_pos, _wnd_pos + _canvas_size, IM_COL32(64, 128, 255, 255), 1.f, 0, 2.f);
            }


            g_draw_list->ChannelsSetCurrent(1); // Background
            // 绘制连线
            drawLink();

            g_draw_list->ChannelsSetCurrent(0); // Background
            // edit node link
            if (g_node_operation == NO_EditingLink) {
                ImVec2 p1 = editingNodeSource;
                ImVec2 p2 = io.MousePos;
                g_draw_list->AddLine(p1, p2, IM_COL32(200, 200, 200, 255), 3.0f);
            }

        }
        return _rel;
    }

    ENGINE_API_THIRDPARTY void endCanvas() {
        auto io = ImGui::GetIO();
        auto drawList = g_draw_list;
        g_view_state->m_mouse_position = io.MousePos;
        drawList->PopClipRect();
        // 设置Node动作状态
        // 移动选中Node
        if (g_node_operation == NO_MovingNodes) {
            if (ImGui::IsMouseDragging(0, 1)) {
                ImVec2 delta = io.MouseDelta / g_view_state->m_factor;
                if (fabsf(delta.x) >= 1.f || fabsf(delta.y) >= 1.f) {
                    if (g_delegate)
                        g_delegate->moveSelectedNodes(delta);
                }
            }
        }
        drawList->ChannelsSetCurrent(2);
        handleQuadSelection();
        handleConnections();
        drawList->ChannelsMerge();

        // 释放所有动作
        if (g_node_operation == NO_PanView) {
            if (!io.MouseDown[2]) {
                g_node_operation = NO_None;
            }
        } else if (g_node_operation != NO_None && !io.MouseDown[0]) {
            g_node_operation = NO_None;
        }


        ImGui::PopStyleColor();
        ImGui::EndGroup();
        ImGui::EndChildFrame();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        // DEBUG
        g_view_state->m_debug_info.m_select_node_index = g_select_node_index;
        g_view_state->m_debug_info.m_hover_node_index = g_hover_node_index;
        g_view_state->m_debug_info.m_hover_slot_index = g_hover_slot_index;
    }

    bool isRectHover(const ImVec2 &_start, const ImVec2 &_size) {
        ImRect _rect = {_start,_start+_size};
        ImVec2 _mouse_pos = g_view_state->m_mouse_position;
        return _rect.Contains(_mouse_pos);
    }
}