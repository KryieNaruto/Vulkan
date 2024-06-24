#pragma once

#ifdef EDITOR_EXPORTS
#define ENGINE_API_EDITOR __declspec(dllexport)
#else
#define ENGINE_API_EDITOR __declspec(dllimport)
#endif

// DragDrop Payload Name
#define DRAG_DROP_PAYLOAD_FILE "drag_drop_payload_file"		// �϶��ļ�


//-----------------------------------------------------
// ��׺
#define SUFFIX_PROJECT_FILE ".zs"	// ��Ŀ��׺
#define SUFFIX_MODEL std::set<std::string>({	\
".gltf",\
".fbx",\
".obj",\
".stl",\
}) // ģ�ͺ�׺

#define SUFFIX_JSON std::set<std::string>({	\
".js",\
".json",\
})	// json

#define SUFFIX_BINARY std::set<std::string>({	\
".bin",\
".mtl",\
".lib",\
".dll",\
".spv",\
})	// ������

#define SUFFIX_TEXT std::set<std::string>({	\
".txt",\
})	// �ı�
#define SUFFIX_SHADER std::set<std::string>({	\
".frag",\
".vert",\
".vs",\
".fs",\
".gemo",\
".gs",\
".hlsl",\
".hlsli",\
".spv",\
})	// ��ɫ��

#define SUFFIX_IMAGE_STR \
".jpg",\
".jpeg",\
".png",\
".gif",\
".hdri",\
".hdr",

#define SUFFIX_IMAGE std::set<std::string>({	\
SUFFIX_IMAGE_STR \
})	// ͼƬ��׺

#define SUFFIX_IMAGE_VEC std::vector<std::string>({	\
SUFFIX_IMAGE_STR \
})	// ͼƬ��׺
