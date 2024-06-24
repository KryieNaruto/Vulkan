#pragma once
#include "Shape.h"
#include "Cube.h"
#include "Sphere.h"
#include "Plane.h"
namespace Core::Resource {
	void draw_shape(uint32_t _shape, SceneObject* _scene, Material* _mat, VkCommandBuffer _cmd, Shader_Data_Binding_Func _func, void* _cam);
	void draw_shape(uint32_t _shape, Material* _mat, VkCommandBuffer _cmd);
	// �������һ��Shape,��ôMaterialҲֻ��һ��������ʾSet������
	ENGINE_API_SCENE Shape* getShape(uint32_t _shape, const std::string& _name);
	ENGINE_API_SCENE Shape* getCube(const std::string& _name = SHAPE_CUBE);
	ENGINE_API_SCENE Shape* getPlane(const std::string& _name = SHAPE_PLANE);
	ENGINE_API_SCENE Shape* getSphere(const std::string& _name = SHAPE_SPHERE);
}