#pragma once
#include <unordered_map>
#include <ThirdParty/vulkan/vulkan.h>
#include <string>
#include <filesystem>
#include <ThirdParty/spirv-reflect/spirv_reflect.h>
#include "core/info/include/application_window_info.h"
namespace ThirdParty {

	enum ENGINE_API_THIRDPARTY ShaderType {
		vertex,
		fragment,
		compute,
		geometry,
	};
	std::string ENGINE_API_THIRDPARTY getShaderTypeStr(ShaderType _type);
	ShaderType ENGINE_API_THIRDPARTY string_to_ShaderType(const std::string& _type);
	/************************************************************************/
	/* Vulkan shader 编译器                                                 */
	/************************************************************************/

	class ENGINE_API_THIRDPARTY ShaderCompiler
	{
	public:
	public:
		// 使用shaderc::glslang编译vkshader，仅支持glsl
		// 输出目录：g_proj_info->m_resource_path/[fileName]/[stage].spv
		// 返回输出路径
		std::vector<std::string> compileVulkanShader(const std::vector<std::string>& _shader_paths, const std::vector<ShaderType>& _shader_types,const std::string& _out_folder = "");
		// 编译单个文件，并指定输出路径, 返回binary
		std::vector<uint32_t> compileVulkanShader(const std::string& _path, const ShaderType& _shader_type, const std::string& _out_path);

		SpvReflectShaderModule createReflectShaderModule(const std::vector<uint32_t>& _data);

		// 反射shader:Debug
		void reflectInfo(const std::string& _shader_name, const std::vector<uint32_t>& _shader_data);
	private:
		std::string getShaderFormatStr(uint32_t _format);
		std::string getShaderDescriptorTypeStr(uint32_t _descriptor_type);
	};

}

