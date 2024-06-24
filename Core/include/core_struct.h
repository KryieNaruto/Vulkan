#pragma once
#include <string>
#include <filesystem>
#include <third_party_manager_global.h>
#include <ThirdParty/json/json.h>
#include <tuple>
#include <unordered_map>
#pragma warning(disable:4996)
#pragma warning(disable:4251)

#ifdef CORE_EXPORTS
#define ENGINE_API_CORE __declspec(dllexport)
#else
#define ENGINE_API_CORE __declspec(dllimport)
#endif // ENGINE_API_CORE


namespace Core {
	enum RESOURCE_TYPE {
		UNDEFINED = 0xFFFFFFFF,
		EDITOR = 0x00000010,
		SHADER = 0x00000100,
		VERTEX_SHADER = 0x00000101,
		GEOMETRY_SHADER = 0x00000102,
		FRAGMENT_SHADER = 0x00000103,
		RMODEL = 0x00000200,
		RMESH = 0x00000201,
		RVERTEX = 0x00000202,
		MODEL = 0x0000203,
		RTEXTURE = 0x00000210,
		RTEXTURE_DIFFUSE = 0x00000211,
		RTEXTURE_SPECULAR = 0x00000212,
		RMATERIAL = 0x00000300,
		RLIGHT = 0x00000400,
		RENDERPASS = 0x00001000,
		SCENE = 0x00002000,
	};

	enum FILE_TYPE {
		NULL_ROOT = 0x0000,
		FOLDER = 0x0001,
		FILE = 0x0010,
	};

	// �ļ���Ϣ
	/// <�ڵ�����>
	/// 0 Input
	/// 1 Output
	///		FilePath: const char*
	struct ENGINE_API_CORE FileInfo {
		FileInfo();
		FileInfo(const std::filesystem::directory_entry&);
		~FileInfo();
		std::string m_fileName;
		std::string m_suffixName;
		std::string m_absolutePath;
		std::string m_date;
		int32_t m_size;
		bool m_rename = false;
		FILE_TYPE m_fileType;

		FileInfo* p_parent_folder;
		bool isFile();
		bool isFolder();
		bool isSuffix(const std::string& suffixName);
		const char* getFileType();
		ThirdParty::imgui::Template* getTemplate();
		// ����
		ThirdParty::imgui::Template* p_template = nullptr;
	};

	// ��Ŀϸ��
	struct ENGINE_API_CORE ProjectInfo {
		ProjectInfo() = default;
		ProjectInfo(FileInfo& fileInfo);
		// д���ļ�
		~ProjectInfo();
		// ���浽jsonȫ��
		void save();
		// obj -> json value
		Json::Value serializeToJSON();
		// json value -> obj
		void deserializeToObj(Json::Value& value);
		inline std::string& getJsonTitle() { return m_json_title; }
		std::string m_name;	// ��Ŀ����
		std::string m_root_path;	// ��Ŀ��·��
		std::string m_file_path;	// ��Ŀ�ļ�·��
		std::string m_resource_path;// ��Դ�ļ���
	private:
		// ���ڵ�����
		std::string m_json_title = "ProjectInfo";
	};


	// ���������ṹ�彫����ڴ�й©
	// δ��������ͷ�
	// ͨ�ýṹ��, �ڳ�������ʵ������Model,Light
	// spirv �ὫStruct �е�bool����ȫ�������int...
	// �����˶�����Σ�������int��Ϊbool��checkBox
	struct ENGINE_API_CORE CommonStruct {
		CommonStruct() = default;
		CommonStruct(const std::string& _name,const std::vector<std::pair<std::string, std::pair<size_t, size_t>>>& _size_offset);
		// ���ƣ���С��ƫ�ƣ����յ�ַ
		std::vector<std::tuple<std::string, size_t, size_t, void*>> m_members;
		std::vector<std::string> m_member_types;
		// name && name_2 && name_3, ����material editor;
		std::unordered_map<std::string, std::pair<std::string, std::string>> m_names;

		ThirdParty::imgui::Template* p_template = nullptr;
		std::string m_struct_name = "CommonStruct";
		virtual void initTemplate();
		virtual void initObjFromTemplate();
	};
	// ���ݽṹ�壬 ���ڳ�������ʵ�������ݽṹ����ڴˣ���bool��float����Input��ֻ��Output
	struct ENGINE_API_CORE DataStruct : CommonStruct {
		DataStruct() = default;
		DataStruct(ThirdParty::SlotData* _p_data);
		virtual ~DataStruct();
		void initTemplate() override;
		void initObjFromTemplate() override;
	};
}