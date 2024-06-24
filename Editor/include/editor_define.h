#pragma once

#ifdef EDITOR_EXPORTS
#define ENGINE_API_EDITOR __declspec(dllexport)
#else
#define ENGINE_API_EDITOR __declspec(dllimport)
#endif

// DragDrop Payload Name
#define DRAG_DROP_PAYLOAD_FILE "drag_drop_payload_file"		// 拖动文件


//-----------------------------------------------------
// 后缀
#define SUFFIX_PROJECT_FILE ".zs"	// 项目后缀
#define SUFFIX_MODEL std::set<std::string>({	\
".gltf",\
".fbx",\
".obj",\
".stl",\
}) // 模型后缀

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
})	// 二进制

#define SUFFIX_TEXT std::set<std::string>({	\
".txt",\
})	// 文本
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
})	// 着色器

#define SUFFIX_IMAGE_STR \
".jpg",\
".jpeg",\
".png",\
".gif",\
".hdri",\
".hdr",

#define SUFFIX_IMAGE std::set<std::string>({	\
SUFFIX_IMAGE_STR \
})	// 图片后缀

#define SUFFIX_IMAGE_VEC std::vector<std::string>({	\
SUFFIX_IMAGE_STR \
})	// 图片后缀
