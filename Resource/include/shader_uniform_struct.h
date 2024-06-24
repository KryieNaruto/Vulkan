#pragma once

/************************************************************************/
// Shader Uniform Block 定义在这里                                                                     
// 将data 与ubo联系在一起，为自定义内容
//	这里是API 1.0 与1.1最大的区别，我将Shader_func加入了一个新的参数_mesh
// 也就是说，2代函数，需要传入mesh参数，这样只需要使用一次addFunc，而不用shader使用一次addFunc，Model也使用一次addFunc
// 至于为什么不把一代以及model的addFunc删除，可能这2个函数之后会有用
/************************************************************************/
#include <ThirdParty/glm/glm.hpp>
#include <ThirdPartyManager/third_party_manager_global.h>
#include <Core/include/core_global.h>
namespace Core::Resource {

}