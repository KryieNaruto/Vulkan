#pragma once
#include "base_editor.h"
#include <Core/include/core_struct.h>
#include "editor_resource.h"
namespace Editor {

class FileEditor :
    public Editor::ZEditor
{
public:
    FileEditor();
    virtual ~FileEditor();
    virtual void onRender();
    virtual void onUpdate();
private:
    // 每行图标个数
    uint32_t m_line_file_count;
    // 指示性图标大小
    int m_child_flag;
    int m_child_flag_without_border;
    int m_window_flag;
    ImVec2 m_child_size;
    ImVec2 m_icon_size;
    ImVec2 m_child_size_small;
    ImVec2 m_icon_size_small;
    // 按钮颜色
    ImVec4 m_hover_color;
    // 待机颜色
    ImVec4 m_stand_color;
    // 禁止颜色
    ImVec4 m_disable_color;
    // 当前文件夹
    Core::FileInfo* p_current_folder = nullptr;
    // 文件夹 上一步
    std::vector<std::string> p_folder_previous;
    // 文件夹 下一步
    std::vector<std::string> p_folder_next;
    // 鼠标指向
    Core::FileInfo* p_folder_hover = nullptr;
    // 鼠标选中文件
    Core::FileInfo* p_folder_choose = nullptr;
    // 当前路径
    std::string m_current_path;
    bool m_openging = false;
    // 清空当前文件列表
    void clearFileList();
    // 更新FileList
    void updateFileList();
    // 绘制图标, 返回是否指向
    bool renderFileIcon(Core::FileInfo* _file_info,uint32_t _file_index);
    // 加载图标资源
    void preloadIcon();
};

}

