#pragma once
#include <string>
#include "scene_inner_global.h"
#include <ThirdParty/imgui/imgui.h>
#include <ThirdPartyManager/third_party_manager_global.h>
#include <ThirdPartyManager/common/include/_debug_func.h>
#include <Resource/resource_global.h>
namespace Core::Resource {

	enum  ENGINE_API_SCENE SCENE_TYPE {
		// -------------------------- MODEL
		MODEL = 0x0010,
		SHAPE,
		PLANE,
		CUBE,
		SPHERE,
		MESH = 0x0020,
		LIGHT = 0x0040,
		// -------------------------- CAMERA
		CAMERA = 0x0060,
		CAMERA_SUB,
		// -------------------------- SCENE
		SCENE = 0x0080,
		// -------------------------- SHADER
		SHADER = 0x00A0,
		MATERIAL,
	};

	//-----------------------------
	// Scene ����
	// ��̳�SceneObject��Ҫ������ڵ�һ�̳���
	// Ȼ���ʼ��ʱ���� setZResourceProperty(p_property); ��ʹ���ܹ�����ZResource����
	//-----------------------------
	class ENGINE_API_SCENE SceneObject 
	{
	public:
		inline static std::unordered_map<std::string, SceneObject*> p_all_scene_obj;
		inline static void saveSceneObject(const std::string& _name, SceneObject* _scene);
		inline static void removeSceneObject(const std::string& _name);
		inline static void removeSceneObject(SceneObject* _obj);
		inline static SceneObject* getSceneObject(const std::string& _name);
	public:
		inline SceneObject(const std::string& _name);
		virtual ~SceneObject() {};

		const std::string& getSceneName() { return name; }
		void setSceneName(const std::string& _name) { name = _name; }
		// ���Ա༭��, ��Editor�Ĺ�����Ͻ�SceneObject
		virtual void propertyEditor() = 0;
		// �Ƿ�����
		bool& getHidden() { return m_is_hidden; }
		// �Ƿ�ѡ��
		bool& getChoose() { return m_is_choose; }
		void setChoose(bool _choose) { m_is_choose = _choose; }
		uint32_t getSceneObejctType() { return m_scene_type; }

		SceneObject* setScene(void* _parent_scene) { p_parent_scene = _parent_scene; return this; }
		void* getScene() { return p_parent_scene; }
		SceneObject* setZResourceProperty(ObjectProperty* _prop) { p_scene_property = _prop; return this; }
		ObjectProperty* getSceneObjectProperty() { return this->p_scene_property; }
		virtual inline void draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func);
	protected:
		std::string name;
		bool m_is_hidden = false;
		bool m_is_choose = false;
		void* p_parent_scene = nullptr;
		uint32_t m_scene_type = SHAPE;
		ObjectProperty* p_scene_property = nullptr;
	};

	SceneObject::SceneObject(const std::string& _name)
	{
		name = Core::Resource::getName(_name);
		ImGui::SetCurrentContext(ThirdParty::imgui_manager.getCurrentContext());
		saveSceneObject(name, this);
	}

	void SceneObject::draw(VkCommandBuffer _cmd, Shader_Data_Binding_Func _func)
	{

	}

	void SceneObject::saveSceneObject(const std::string& _name, SceneObject* _scene)
	{
		p_all_scene_obj[_name] = _scene;
	}

	void SceneObject::removeSceneObject(const std::string& _name)
	{
		auto _ite = p_all_scene_obj.find(_name);
		if (_ite != p_all_scene_obj.end()) {
			p_all_scene_obj.erase(_ite);
		}
	}

	void SceneObject::removeSceneObject(SceneObject* _obj)
	{
		for (const auto [_name, _o] : p_all_scene_obj) {
			if (_o == _obj) p_all_scene_obj.erase(p_all_scene_obj.find(_name));
			break;
		}
	}

	Core::Resource::SceneObject* SceneObject::getSceneObject(const std::string& _name)
	{
		auto _ite = p_all_scene_obj.find(_name);
		if (_ite != p_all_scene_obj.end()) {
			return _ite->second;
		}
		else return nullptr;
	}

}