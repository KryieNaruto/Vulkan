#include "editor_global.h"

namespace Editor {

	void loadEditors()
	{
		// RenderPass Ԥ��
		Core::Resource::preload();
		editor = new Editor;
		editor_root = new ZEditor(true); {
			editor_root->setOnRenderFunc(OnRenderFunc::editor_root_func);	
		}
		editor_scene = new SceneEditor;
		file_editor = new FileEditor;
		material_editor = new MaterialEditor;
	}

	// ����������
	ENGINE_API_EDITOR Editor* editor = nullptr;
	// ���ڵ�
	ZEditor* editor_root = nullptr;
	// Scene Editor
	SceneEditor* editor_scene = nullptr;
	// File Editor
	FileEditor* file_editor = nullptr;
	// Material Editor
	MaterialEditor* material_editor = nullptr;


}