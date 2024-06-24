#pragma once
#include "include/base_editor.h"
#include "editor.h"
#include "include/scene_editor.h"
#include "include/file_editor.h"
#include "include/material_editor.h"
#include "editor_global_func.h"
namespace Editor {
	// ����ȫ��editor
	ENGINE_API_EDITOR void loadEditors();
	// ȫ��editor
	// ����������
	__declspec(dllimport) extern Editor* editor;
	// ���ڵ�
	extern ZEditor* editor_root;
	// Scene Editor
	extern SceneEditor* editor_scene;
	// File Editor
	extern FileEditor* file_editor;
	// Material Editor
	extern MaterialEditor* material_editor;
}