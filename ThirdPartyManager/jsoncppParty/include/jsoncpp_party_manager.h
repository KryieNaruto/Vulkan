#pragma once
#include "common/include/third_party_manager.h"
#include <json/json.h>
namespace ThirdParty {

	typedef struct ENGINE_API_THIRDPARTY JsonWriter {
		JsonWriter();
		virtual ~JsonWriter();
		Json::FastWriter* p_writer = nullptr;
		Json::Value* p_value = nullptr;
	}JsWriter;

	typedef struct ENGINE_API_THIRDPARTY JsonReader {
		JsonReader();
		virtual ~JsonReader();
		Json::Reader* p_reader = nullptr;
		Json::Value* p_value = nullptr;
	}JsReader;

	class ENGINE_API_THIRDPARTY JsoncppManager
		:public Manager<JsoncppManager>
	{
	public:
		JsoncppManager() = default;
		virtual ~JsoncppManager();

		virtual JsoncppManager* init() override;
		virtual void destroy() override;
		virtual void windowResize() override;

		// 保存json到文件
		void save(const std::string& json_file_path);

		// 获取json value节点
		Json::Value& getValue(const std::string& json_path);
		
		// 将Object序列化后，添加value节点， obj -> json
		template <class T>
		JsoncppManager* addValue(const std::string& json_path, T* _Tx);
		
		// 反序列化 从文件中加载value ,Json-> value
		JsoncppManager* addValueFromFile(const std::string& json_path);

		// 反序列化 从Json Value[json_path]加载obj
		template<class T>
		T* getObjectFromFile(const std::string& json_path);

		// 清除json 和value
		JsoncppManager* clear();
	private:
		// 直接添加value节点
		JsoncppManager* addJsonValue(const std::string& json_path, Json::Value value);
		// 读取json 从[json_file_path]中
		std::string read(const std::string& json_file_path);
		// 读取json 从g_file_json[json_file_path]中
		std::string& get(const std::string& json_file_path);
		// 写入json 到文件[json_file_path]
		std::string write(const std::string& json_file_path);


	private:
		JsWriter* p_writer = nullptr;
		JsReader* p_reader = nullptr;

		// 根据文件路径索引，Value为根节点，后续所有节点均在Value[m_json_title]上
		std::map<std::string, Json::Value> m_values;
	protected:
	};

	template<class T>
	T* ThirdParty::JsoncppManager::getObjectFromFile(const std::string& json_path)
	{
		T* _t = new T;
		_t->deserializeToObj(getValue(json_path)[_t->getJsonTitle()]);
		return _t;
	}

	template <class T>
	JsoncppManager* ThirdParty::JsoncppManager::addValue(const std::string& json_path, T* _Tx)
	{
		auto& _root_value = getValue(json_path);
		Json::Value value = _Tx->serializeToJSON();
		_root_value[_Tx->getJsonTitle()] = value;
		return this;
	}
}

