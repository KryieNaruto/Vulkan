#include "jsoncpp_party_manager.h"
#include "global_info.h"
#include <fstream>
#include "debug_to_console.h"

ThirdParty::JsoncppManager::~JsoncppManager()
{
	if (p_writer) delete p_writer;
	if (p_reader) delete p_reader;
}

ThirdParty::JsoncppManager* ThirdParty::JsoncppManager::init()
{
	this->p_writer = new JsWriter;
	this->p_reader = new JsReader;

	this->m_isInitialized = true;

	return this;
}

void ThirdParty::JsoncppManager::destroy()
{
	if (p_writer) delete p_writer;
	if (p_reader) delete p_reader;
}

void ThirdParty::JsoncppManager::windowResize()
{
	this->m_isInitialized = true;
}

void ThirdParty::JsoncppManager::save(const std::string& json_file_path)
{
	this->get(json_file_path) = this->p_writer->p_writer->write(this->getValue(json_file_path));	// value -> json
	this->write(json_file_path);	// Ð´ÈëÎÄ¼þ
}

ThirdParty::JsoncppManager* ThirdParty::JsoncppManager::addJsonValue(const std::string& json_path, Json::Value value)
{
	this->m_values.insert(std::make_pair(json_path, value));
	return this;
}

ThirdParty::JsoncppManager* ThirdParty::JsoncppManager::addValueFromFile(const std::string& json_path)
{
	const auto& str = read(json_path);
	if (str == "error") return this;
	if (this->p_reader->p_reader->parse(str, *this->p_reader->p_value)) {
		this->addJsonValue(json_path, this->p_reader->p_value);
	}
	else {
		if(str == "")
			this->addJsonValue(json_path, Json::Value());
		else 
			ThirdParty::Core::warn(("Failed to load json file:" + json_path).c_str());
	}
	return this;
}

ThirdParty::JsoncppManager* ThirdParty::JsoncppManager::clear()
{
	ThirdParty::Core::g_json_files.clear();
	this->m_values.clear();
	return this;
}

std::string ThirdParty::JsoncppManager::read(const std::string& json_file_path)
{
	std::ifstream input(json_file_path.c_str(), std::ios::binary);
	if (!input.is_open()) {
		ThirdParty::Core::error(("Failed to load json file:" + json_file_path).c_str());
		return "error";
	}
	std::istreambuf_iterator<char> buf(input);
	const auto& str = std::string(buf, std::istreambuf_iterator<char>());
	ThirdParty::Core::g_json_files.insert(std::make_pair(json_file_path, str));
	input.close();
	return str;
}

std::string& ThirdParty::JsoncppManager::get(const std::string& json_file_path)
{
	return ThirdParty::Core::g_json_files.find(json_file_path)->second;
}

std::string ThirdParty::JsoncppManager::write(const std::string& json_file_path)
{
	std::ofstream output(json_file_path, std::ios::out);
	if (!output.is_open()) {
		ThirdParty::Core::error(("Failed to create json file:" + json_file_path).c_str());
		return "";
	}
	const auto& str = this->get(json_file_path);
	output << str;
	output.close();
	return str;
}

Json::Value& ThirdParty::JsoncppManager::getValue(const std::string& json_path)
{
	const auto& ite = this->m_values.find(json_path);
	if (ite != m_values.end()) return ite->second;
	Json::Value root;
	this->m_values.insert(std::make_pair(json_path, root));
	return this->m_values.find(json_path)->second;
}

ThirdParty::JsonWriter::JsonWriter()
{
	if (!p_writer) p_writer = new Json::FastWriter;
	if (!p_value) p_value = new Json::Value;
}

ThirdParty::JsonWriter::~JsonWriter()
{
	if (p_writer) delete p_writer;
	if (p_value) delete p_value;
}

ThirdParty::JsonReader::JsonReader()
{
	if (!p_reader) p_reader = new Json::Reader;
	if (!p_value) p_value = new Json::Value;
}

ThirdParty::JsonReader::~JsonReader()
{
	if (p_reader) delete p_reader;
	if (p_value) delete p_value;
}
