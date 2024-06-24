#include "vkobject/vk_shader_compiler.h"
#include <ThirdParty/shaderc/shaderc.hpp>
#include <ThirdParty/spirv-reflect/spirv_reflect.h>
#include <filesystem>
#include <fstream>
#include "utils/utils.h"
#include <cassert>

#define SWITCH_RETURN(value) case value: return #value;

std::vector<std::string> ThirdParty::ShaderCompiler::compileVulkanShader(const std::vector<std::string>& _shader_paths, const std::vector<ShaderType>& _shader_types, const std::string& _out_folder)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	std::vector<std::string> _out;
	// 设置目标版本
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
	options.SetTargetSpirv(shaderc_spirv_version_1_6);

	const bool optimize = true;
	if (optimize) {
		// 编译器级别定级:优化性能
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
	}

	// 编译shader
	int index = 0;
	for (auto& stage : _shader_types) {
		auto path_index = index++;
		if (path_index >= _shader_paths.size()) path_index -= 1;
		const auto path = _shader_paths[path_index];
		const auto& source = Utils::readFile(path);
		const auto& fileName_with_suffix = std::filesystem::path(path).filename().string();
		auto fileName = std::filesystem::path(path).replace_extension().filename().string();
		// 编译
		shaderc::SpvCompilationResult module =
			compiler.CompileGlslToSpv(
				source,
				(shaderc_shader_kind)stage,
				fileName.c_str(),
				options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			Core::error(module.GetErrorMessage().c_str());
			continue;
		}
		// 输出文件
		std::string out_path;
		std::string out_folder;
		// 创建文件夹
		if (_out_folder.empty() || _out_folder == "")
			out_folder = path.substr(0, path.find(fileName_with_suffix)) + "\\" + fileName;
		else
			out_folder = _out_folder + "\\" + fileName;
		std::filesystem::create_directory(out_folder);
		// 如果不设置out_folder,则在glsl文件所在目录生成out_path/[filename]/[stage].spv
		if (_out_folder.empty() || _out_folder == "") {
			out_path = out_folder + "\\" + getShaderTypeStr(stage) + ".spv";
		}
		// 否则生成在out_path/[filename]/[stage].spv
		else {
			out_path = out_folder + "\\" + getShaderTypeStr(stage) + ".spv";
		}
		std::vector<uint32_t> shaderData(module.cbegin(), module.cend());
		std::ofstream out(out_path, std::ios::out | std::ios::binary);
		if (out.is_open())
		{
			auto& data = shaderData;
			out.write((char*)data.data(), data.size() * sizeof(uint32_t));
			out.flush();
			out.close();
			_out.push_back(out_path);
		}
		else {
			Core::warn("Failed to out spirv file:", out_path);
		}
	}
	return _out;
}

SpvReflectShaderModule ThirdParty::ShaderCompiler::createReflectShaderModule(const std::vector<uint32_t>& _data)
{
	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(VECTOR_SIZEOF(_data), _data.data(), &module);
	if (result != SPV_REFLECT_RESULT_SUCCESS) {
		Core::error("Failed to reflect current shader.");
		return {};
	}
	return module;
}

std::vector<uint32_t> ThirdParty::ShaderCompiler::compileVulkanShader(const std::string& _path, const ShaderType& _shader_type, const std::string& _out_path)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	// 设置目标版本
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

	const bool optimize = false;
	if (optimize) {
		// 编译器级别定级:优化性能
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
	}
	const auto& fileName = std::filesystem::path(_path).replace_extension().filename().string();
	// 编译
	shaderc::SpvCompilationResult module =
		compiler.CompileGlslToSpv(
			Utils::readFile(_path),
			(shaderc_shader_kind)_shader_type,
			fileName.c_str(),
			options);

	if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
		Core::error(module.GetErrorMessage().c_str());
		return {};
	}
	// 输出
	// 创建文件夹
	auto str = _out_path;
	str = str.substr(0, _out_path.find_last_of("/\\"));
	std::filesystem::create_directories(str);
	std::vector<uint32_t> shaderData(module.cbegin(), module.cend());
	std::ofstream out(_out_path, std::ios::out | std::ios::binary);
	auto& data = shaderData;
	if (out.is_open())
	{
		out.write((char*)data.data(), data.size() * sizeof(uint32_t));
		out.flush();
		out.close();
	}
	else {
		Core::warn("Failed to out spirv file:", _out_path);
	}
	return data;
}

void ThirdParty::ShaderCompiler::reflectInfo(const std::string& _shader_name, const std::vector<uint32_t>& _shader_data)
{
	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(VECTOR_SIZEOF(_shader_data), _shader_data.data(), &module);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	Core::info("-----------------------------------------------------");
	Core::info("Shader Name:%s", _shader_name.c_str());
	// 输入---------------------
	uint32_t var_count = 0;
	result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
	SpvReflectInterfaceVariable** input_vars = (SpvReflectInterfaceVariable**)malloc(var_count * sizeof(SpvReflectInterfaceVariable*));
	result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
	Core::info("<%d Input Variables>", var_count);
	for (size_t index = 0; index < var_count; index++) {
		Core::info("{");
		Core::info("\tlocation: %d", input_vars[index]->location);
		Core::info("\tformat: %s", getShaderFormatStr(input_vars[index]->format).c_str());
		Core::info("}");
	}
	free(input_vars);
	// Set---------------------
	spvReflectEnumerateDescriptorSets(&module,&var_count,nullptr);
	SpvReflectDescriptorSet** sets = (SpvReflectDescriptorSet**)malloc(var_count * sizeof(SpvReflectDescriptorSet*));
	spvReflectEnumerateDescriptorSets(&module,&var_count,sets);
	Core::info("<%d Descriptor Sets>", var_count);
	for (size_t index = 0; index < var_count; index++) {
		Core::info("Set %d {",sets[index]->set);
			Core::info("\t<%d Descriptor Binding>", sets[index]->binding_count);
			// bindings
			const auto& bindings = sets[index]->bindings;
			for (size_t binding = 0; binding < sets[index]->binding_count; binding++) {
				Core::info("\tBinding: %d {",bindings[binding]->binding);
				Core::info("\t\tName: %s", bindings[binding]->name);
				Core::info("\t\tCount: %d", bindings[binding]->count);
				Core::info("\t\tType: %s", getShaderDescriptorTypeStr(bindings[binding]->descriptor_type).c_str());
				Core::info("\t}");
			}

		Core::info("}");
	}
	free(sets);
	// Binding---------------------

	Core::info("-----------------------------------------------------");

	spvReflectDestroyShaderModule(&module);
}

std::string ThirdParty::ShaderCompiler::getShaderFormatStr(uint32_t _format)
{
	switch ((SpvReflectFormat)_format)
	{
	SWITCH_RETURN(SPV_REFLECT_FORMAT_UNDEFINED);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16B16_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16B16_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16B16_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16B16A16_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16B16A16_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32B32_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32B32_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32B32_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32B32A32_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32B32A32_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64B64_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64B64_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64B64_SFLOAT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64B64A64_UINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64B64A64_SINT);
	SWITCH_RETURN(SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT);
	default:
		return "SPV_REFLECT_FORMAT_UNDEFINED";
		break;
	}
}

std::string ThirdParty::ShaderCompiler::getShaderDescriptorTypeStr(uint32_t _descriptor_type)
{
	switch (SpvReflectDescriptorType(_descriptor_type))
	{
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
	SWITCH_RETURN(SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
	default:
		return "";
	}
}

std::string ThirdParty::getShaderTypeStr(ShaderType _type)
{
	switch (_type)
	{
	case ThirdParty::vertex:
		return "vert";
	case ThirdParty::geometry:
		return "geom";
	case ThirdParty::fragment:
		return "frag";
	default:
		return "shader";
	}
}

ThirdParty::ShaderType ThirdParty::string_to_ShaderType(const std::string& _type)
{
	if (_type == "vertex")
		return ShaderType::vertex;
	if (_type == "geometry")
		return ShaderType::geometry;
	if (_type == "fragment")
		return ShaderType::fragment;
}
