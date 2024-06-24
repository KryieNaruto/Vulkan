#include "sub/shape/shape_static_func.h"

void Core::Resource::draw_shape(
	uint32_t _shape, SceneObject* _scene, Material* _mat, VkCommandBuffer _cmd, Shader_Data_Binding_Func _func, void* _camera) {
	if (_scene && _mat && _func) {
		Shape* _obj = nullptr;
		switch (_shape)
		{
		case CUBE:
			_obj = getCube();
			break;
		case SPHERE:
			_obj = getSphere();
			break;
		case PLANE:
			_obj = getPlane();
			break;
		default:
			break;
		}
		_obj->draw(_scene, _mat, _cmd, _func, _camera);
	}
}

void Core::Resource::draw_shape(uint32_t _shape, Material* _mat, VkCommandBuffer _cmd)
{
	Shape* _obj = nullptr;
	switch (_shape)
	{
	case CUBE:
		_obj = getCube();
		break;
	case SPHERE:
		_obj = getSphere();
		break;
	case PLANE:
		_obj = getPlane();
		break;
	default:
		break;
	}
	_obj
		->setMaterial(_mat)
		->draw(_cmd);
}

Core::Resource::Shape* Core::Resource::getCube(const std::string& _name)
{
	auto _obj = (Shape*)SceneObject::getSceneObject(_name);
	if (!_obj) _obj = new Cube(_name);
	return _obj;
}

Core::Resource::Shape* Core::Resource::getPlane(const std::string& _name)
{
	auto _obj = (Shape*)SceneObject::getSceneObject(_name);
	if (!_obj) {_obj = new Plane(_name);}
	return _obj;
}

Core::Resource::Shape* Core::Resource::getSphere(const std::string& _name)
{
	auto _obj = (Shape*)SceneObject::getSceneObject(_name);
	if (!_obj) _obj = new Sphere(_name);
	return _obj;
}

Core::Resource::Shape* Core::Resource::getShape(uint32_t _shape, const std::string& _name)
{
	Shape* _obj = nullptr;
	switch (_shape)
	{
	case CUBE:
		_obj = getCube(_name);
		break;
	case SPHERE:
		_obj = getSphere(_name);
		break;
	case PLANE:
		_obj = getPlane(_name);
		break;
	default:
		break;
	}
	return _obj;
}

