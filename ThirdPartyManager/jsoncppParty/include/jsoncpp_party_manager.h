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

		// ����json���ļ�
		void save(const std::string& json_file_path);

		// ��ȡjson value�ڵ�
		Json::Value& getValue(const std::string& json_path);
		
		// ��Object���л������value�ڵ㣬 obj -> json
		template <class T>
		JsoncppManager* addValue(const std::string& json_path, T* _Tx);
		
		// �����л� ���ļ��м���value ,Json-> value
		JsoncppManager* addValueFromFile(const std::string& json_path);

		// �����л� ��Json Value[json_path]����obj
		template<class T>
		T* getObjectFromFile(const std::string& json_path);

		// ���json ��value
		JsoncppManager* clear();
	private:
		// ֱ�����value�ڵ�
		JsoncppManager* addJsonValue(const std::string& json_path, Json::Value value);
		// ��ȡjson ��[json_file_path]��
		std::string read(const std::string& json_file_path);
		// ��ȡjson ��g_file_json[json_file_path]��
		std::string& get(const std::string& json_file_path);
		// д��json ���ļ�[json_file_path]
		std::string write(const std::string& json_file_path);


	private:
		JsWriter* p_writer = nullptr;
		JsReader* p_reader = nullptr;

		// �����ļ�·��������ValueΪ���ڵ㣬�������нڵ����Value[m_json_title]��
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

