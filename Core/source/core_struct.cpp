#include "core_struct.h"
#include <filesystem>
#include <format>
#include <comutil.h>
#include <ThirdPartyManager/third_party_manager_global.h>

#pragma comment(lib, "comsuppw.lib")
namespace Core {

	FileInfo::FileInfo(const std::filesystem::directory_entry& fileEntry)
	{
		if (fileEntry.is_directory()) {
			// 文件夹
			this->m_fileType = FILE_TYPE::FOLDER;
		}
		else
		{
			// 文件
			this->m_fileType = FILE_TYPE::FILE;
			this->m_suffixName = fileEntry.path().extension().string();
			this->m_size = fileEntry.file_size();
		}
		//-------------------
		_bstr_t wstr = fileEntry.path().filename().wstring().c_str() + '\0';
		char* p = (char*)wstr;
		this->m_fileName = p;
		//-------------------
		wstr = std::filesystem::absolute(fileEntry.path()).wstring().c_str() + '\0';
		p = (char*)wstr;
		this->m_absolutePath = p;
		//-------------------
		if (this->m_fileName.empty())
			this->m_fileName = this->m_absolutePath;
		//-------------------
		// 获取最后修改时间
		struct _stat _st; _stat(this->m_absolutePath.c_str(), &_st);
		struct tm _tm{};
		_localtime64_s(&_tm, &_st.st_mtime);
		char buf[30];
		::strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &_tm);
		this->m_date = buf;
		//-------------------
		p_parent_folder = nullptr;

		p_template = ThirdParty::imgui::Template::newTemplate();
		{
			auto _output = new ThirdParty::imgui::SlotTemplateOutput;
			_output->setName(m_fileName);
			_output->p_data = &this->m_absolutePath;
			p_template->p_outputs.push_back(_output);
		}
	}

	FileInfo::~FileInfo() {
		if(p_template) ThirdParty::imgui::Template::deleteTemplate(p_template);
	}

	FileInfo::FileInfo()
	{
		this->m_fileType = FILE_TYPE::NULL_ROOT;
		this->m_suffixName = "";
		this->m_size = -1;
		//-------------------
		this->m_fileName = "";
		//-------------------
		this->m_absolutePath = "";
		//-------------------
		// 获取最后修改时间
		this->m_date = "";
		//-------------------
		p_parent_folder = NULL;
	}

	bool FileInfo::isFile()
	{
		return (this->m_fileType & FILE_TYPE::FILE) == FILE_TYPE::FILE;
	}

	bool FileInfo::isFolder()
	{
		return (this->m_fileType & FILE_TYPE::FOLDER) == FILE_TYPE::FOLDER;
	}

	bool FileInfo::isSuffix(const std::string& suffixName)
	{
		return strcmp(this->m_suffixName.c_str(), suffixName.c_str()) == 0;
	}

	const char* FileInfo::getFileType()
	{
		switch (this->m_fileType)
		{
		case FILE_TYPE::FOLDER:	return "文件夹";
		case FILE_TYPE::FILE:	return "文件";
		default:				return "UnKnown";
		}
	}

	ThirdParty::imgui::Template * FileInfo::getTemplate() {
		return p_template;
	}

	ProjectInfo::ProjectInfo(FileInfo& fileInfo)
	{
		this->m_name = fileInfo.m_fileName.substr(0, fileInfo.m_fileName.find_last_of("."));
		this->m_file_path = fileInfo.m_absolutePath;
		if (fileInfo.p_parent_folder)
			this->m_root_path = fileInfo.p_parent_folder->m_absolutePath;
		else this->m_root_path = fileInfo.m_absolutePath.substr(0, fileInfo.m_absolutePath.find_last_of("/\\"));
		this->m_resource_path = this->m_root_path + "\\" + "resources";
	}

	ProjectInfo::~ProjectInfo()
	{
		save();
	}

	void ProjectInfo::save()
	{
		const auto& json_str = ThirdParty::jsoncpp_manager.addValue<ProjectInfo>(this->m_file_path, this);
	}

	Json::Value ProjectInfo::serializeToJSON()
	{
		Json::Value sub;
		sub["m_name"] = m_name;
		sub["m_file_path"] = m_file_path;
		sub["m_root_path"] = m_root_path;
		sub["m_resource_path"] = m_resource_path;

		return sub;
	}

	void ProjectInfo::deserializeToObj(Json::Value& value)
	{
		this->m_name = value["m_name"].asString();
		this->m_file_path = value["m_file_path"].asString();
		this->m_root_path = value["m_root_path"].asString();
		this->m_resource_path = value["m_resource_path"].asString();
		if (m_resource_path.empty()) {
			m_resource_path = m_root_path + "\\" + "resources";
		}
	}


	CommonStruct::CommonStruct(const std::string& _name,const std::vector<std::pair<std::string, std::pair<size_t, size_t>>>& _size_offset)
	{
		m_struct_name = _name;
		for (auto _member : _size_offset) {
			m_members.push_back(std::make_tuple(_member.first, _member.second.first, _member.second.second, nullptr));
		}
	}

	void CommonStruct::initTemplate()
	{
		auto members = m_members;
		if (!p_template) p_template = new ThirdParty::imgui::Template();
		for (auto member : members)
		{
			auto _input = new ThirdParty::imgui::SlotTemplateInput();
			_input->setName(std::get<0>(member));
			p_template->addInputSlotTemplate(_input);
		}

		// 设置输出
		{
			auto _output = new ThirdParty::imgui::SlotTemplateOutput();
			_output->setName(m_struct_name);
			_output->setName2(m_struct_name);
			_output->setName3("CommonStruct");
			_output->p_data = this;	// 用于记录多个input的name_2
			p_template->addOutputSlotTemplate(_output);
		}
	}

	void CommonStruct::initObjFromTemplate()
	{
		auto _inputs = p_template->p_inputs;
		int i = 0;
		for (auto _input : _inputs) {
			if (_input->m_name_2) {
				m_names[std::get<0>(m_members[i])].first = _input->m_name_2;
			}
			if (_input->m_name_3) {
				m_names[std::get<0>(m_members[i])].second = _input->m_name_3;
			}
			std::get<3>(m_members[i]) = _input->p_data;
			i++;
		}
	}

	DataStruct::DataStruct(ThirdParty::SlotData* _p_data)
		:CommonStruct(_p_data->getMemberStructOrdered().first, _p_data->getMemberStructOrdered().second)
	{
		if (ImGui::GetCurrentContext() == nullptr)
			ImGui::SetCurrentContext(ThirdParty::imgui_manager.getCurrentContext());
		// 设定成员类型。
		auto _struct_members = _p_data->getMemberStruct();
		for (auto _member_ite : _struct_members.second) {
			auto _struct_type = _p_data->getMemberType(_member_ite.first);
			m_member_types.push_back(_struct_type);
		}
	}

	DataStruct::~DataStruct()
	{
		for (auto _member : this->m_members)
			if (std::get<3>(_member) != nullptr) {
				free(std::get<3>(_member));
				std::get<3>(_member) = nullptr;
			}
	}

	void DataStruct::initTemplate()
	{
		if (!p_template) p_template = new ThirdParty::imgui::Template();
		// Output为数据段
		// Output
		auto _output = new ThirdParty::imgui::SlotTemplateOutput();
		_output->setName(m_struct_name);
		_output->setName2(m_struct_name);
		_output->setName3("DataStruct");
		_output->p_data = this;
		p_template->addOutputSlotTemplate(_output);

		std::vector<std::function<void(void)>> _funcs;
		// 根据MemberTypes 初始化成员数据
		for (int i = 0; i < m_member_types.size(); i++) {
			auto& _member = m_members[i];
			auto& _type = m_member_types[i];
			std::function<void(void)> _func = NULL;
			if (_type == "bool" || _type == "int") {
				std::get<3>(_member) = malloc(std::get<1>(_member));
				*((int*)std::get<3>(_member)) = false;		// glsl bool = int字长。
				_func = [this,_member]()->void {
					ImGui::Checkbox(std::get<0>(_member).c_str(), (bool*)std::get<3>(_member));
				};
			}
			/*else if (_type == "int") {
				std::get<3>(_member) = new int(0);
				_func = [this, _member]()->void {
					ImGui::DragInt(std::get<0>(_member).c_str(), (int*)std::get<3>(_member));
					};
			}*/
			else if (_type == "float") {
				std::get<3>(_member) = malloc(std::get<1>(_member));
				*((float*)std::get<3>(_member)) = 0.0f;
				_func = [_output, _member]()->void {
					auto _width = _output->m_final_size.x - ImGui::GetFont()->CalcTextSizeA(ImGui::GetFont()->FontSize, FLT_MAX, 0.0f, std::get<0>(_member).c_str()).x;
					ImGui::SetNextItemWidth(_width);
					ImGui::DragFloat(std::get<0>(_member).c_str(), (float*)std::get<3>(_member),0.1f);
					};
			}
			_funcs.push_back(_func);
		}
		_output->m_functional = [_funcs]()->void {
			if (ImGui::CollapsingHeader("Details")) {
				for (auto _func : _funcs)
					_func();
			}
			};
	}

	void DataStruct::initObjFromTemplate()
	{

	}

}