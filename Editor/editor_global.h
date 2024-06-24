#pragma once
#include "include/base_editor.h"
#include "editor.h"
#include "include/scene_editor.h"
#include "include/file_editor.h"
#include "include/material_editor.h"
#include "editor_global_func.h"
namespace Editor {
	// 加载全局editor
	ENGINE_API_EDITOR void loadEditors();
	// 全局editor
	// 最终主窗体
	__declspec(dllimport) extern Editor* editor;
	// 根节点
	extern ZEditor* editor_root;
	// Scene Editor
	extern SceneEditor* editor_scene;
	// File Editor
	extern FileEditor* file_editor;
	// Material Editor
	extern MaterialEditor* material_editor;
}