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
    // ÿ��ͼ�����
    uint32_t m_line_file_count;
    // ָʾ��ͼ���С
    int m_child_flag;
    int m_child_flag_without_border;
    int m_window_flag;
    ImVec2 m_child_size;
    ImVec2 m_icon_size;
    ImVec2 m_child_size_small;
    ImVec2 m_icon_size_small;
    // ��ť��ɫ
    ImVec4 m_hover_color;
    // ������ɫ
    ImVec4 m_stand_color;
    // ��ֹ��ɫ
    ImVec4 m_disable_color;
    // ��ǰ�ļ���
    Core::FileInfo* p_current_folder = nullptr;
    // �ļ��� ��һ��
    std::vector<std::string> p_folder_previous;
    // �ļ��� ��һ��
    std::vector<std::string> p_folder_next;
    // ���ָ��
    Core::FileInfo* p_folder_hover = nullptr;
    // ���ѡ���ļ�
    Core::FileInfo* p_folder_choose = nullptr;
    // ��ǰ·��
    std::string m_current_path;
    bool m_openging = false;
    // ��յ�ǰ�ļ��б�
    void clearFileList();
    // ����FileList
    void updateFileList();
    // ����ͼ��, �����Ƿ�ָ��
    bool renderFileIcon(Core::FileInfo* _file_info,uint32_t _file_index);
    // ����ͼ����Դ
    void preloadIcon();
};

}

