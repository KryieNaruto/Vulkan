#pragma once
#include "ZResource.h"
#include "RMaterial.h"
#include "RVertex.h"
#include "RMesh.h"
#include "RTexture.h"
#include <numeric>
#include <thread>
#include <mutex>
#define GLM_ENABLE_EXPERIMENTAL
#include <ThirdParty/glm/gtx/hash.hpp>

namespace std {
	template<typename F, typename ...T> inline size_t hash_combine(const F& first, const T&... params) {
		if constexpr (sizeof...(params) == 0) {
			return std::hash<F>()(first);
		}
		else {
			size_t seed = hash_combine(params...);
			return std::hash<F>()(first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	}
	template<typename A, typename B>
	struct std::hash<std::pair<A, B>> {
		inline size_t operator() (const std::pair<A, B>& pr) const {
			return hash_combine(pr.first, pr.second);
		}
	};
	template<typename ...T>
	struct std::hash<std::tuple<T...>> {
		inline size_t operator() (const std::tuple<T...>& tp) const {
			return apply(hash_combine<T...>, tp); // 需要C++17
		}
	};


    template<> struct hash<Core::Resource::RVertex*> {
        size_t operator()(Core::Resource::RVertex* const& _v) const;
    };

	template<> struct hash<std::vector<Core::Resource::RTexture*>> {
		bool operator() (std::vector<Core::Resource::RTexture*> const& _tex) const {
			return std::accumulate(_tex.begin(), _tex.end(), 0, [](std::size_t _hash, Core::Resource::RTexture* _t) {
				return std::hash_combine(_hash, std::hash<std::string>()(_t->getPath()));
				});
		}
	};
}
struct equal {
	bool operator()(const Core::Resource::RVertex* _lhs, const Core::Resource::RVertex* _rhs) const;
};

namespace Core::Resource{
    
    /************************************************************************/
    // 模型数据的第一次加工，Assimp数据->std::vector<xxx>,同时存入vkbuffer
    // 包含所有的顶点数据， RMesh中只记录索引
    /************************************************************************/

    class ENGINE_API_RESOURCE RModel :
        public ZResource
    {
    public:
        RModel();
        RModel(const std::string& _path);
		virtual ~RModel();
		virtual Json::Value serializeToJSON() override;
		virtual void deserializeToObj(Json::Value& _root) override;
        //------------------------------------------------------------------

        // 设置shader 并更具shader创建Material
        virtual RModel* setShader(Shader* _shader = nullptr);
        RMaterial* getRMaterial(uint32_t _mesh_index) { return p_materials[_mesh_index]; }
        const std::vector<RMaterial*>& getRMaterials() { return p_materials; }
        const std::vector<RMesh*>& getRMeshes() { return p_meshes; }
        ThirdParty::VertexBuffer* getVertexBuffer() { return p_vertex_buffer; }

        // 设置为其他类的成员
        void setParent(void* _parent) { p_parent = _parent; }
        void* isMember() { return p_parent; }
        Shader* getShader() { return p_shader; }

        const std::string& getName() { return m_res_name; }
        RMeshProperty* getProperty() { return p_property; }
    protected:
        std::string m_path = "";
        std::string m_directroy = "";
        // 加工assimp数据
        virtual void setup();
        // 根据不同的shader vertex input binding 创建不同的vkbuffer
        virtual void setupVkData();
        virtual void setupVkData(bool _only_vertex_buffer,VERTEX_INPUT_FLAG _input, uint32_t _stride);
    protected:
        const aiScene* p_root_scene = nullptr;
        
        ThirdParty::VertexBuffer* p_vertex_buffer;
		void* p_vertices_in_shader = nullptr;
        
        inline static std::unordered_map<VERTEX_INPUT_FLAG, std::unordered_map<RModel*, ThirdParty::VertexBuffer*>> p_vertex_buffers;
        inline static std::unordered_map<VERTEX_INPUT_FLAG, std::unordered_map<RModel*, void*>> p_vertices_in_shaders;
        
        std::unordered_map<RVertex*, uint32_t, std::hash<RVertex*>, equal> p_unique_vertices;   // 去重顶点
        std::vector<RVertex*> p_vertices;
        std::vector<RMesh*> p_meshes;
        std::unordered_map<uint32_t, std::vector<RTexture*>> p_textures;
        // 合并去重后的Material
        std::unordered_map<uint32_t, std::vector<RTexture*>> p_merge_textures;
        // p_tex转p_merge_tex, [MaterialIndex] [MergeMaterialIndex]
        std::unordered_map<uint32_t, uint32_t> p_texture_To_merge_textures;
        // 材质
        Shader* p_shader = nullptr;
        std::vector<RMaterial*> p_materials; // 上述材质的复制
        RMeshProperty* p_property = nullptr;
        /// 辅助函数
        void init_0();  // 初始化一些父类的东西
        void processAllVertex();        // 读取scene中的所有vertex
        void processAllMaterial();      // 将scene中的纹理数据全部读出保存
        void processNode(aiNode* node); // 处理node
        void mergeAllMaterial();        // 合并相同纹理
        void loadFromFile(const std::string& _path);
        void loadFromModel(const RModel* _p);
        std::vector<RTexture*> loadMaterial(aiMaterial* mat, aiTextureType _type);

        // 是否是其他类的成员
        void* p_parent = nullptr;
        // 是否是从其他Model类加载
        bool m_is_copy = false;
        //=========================================================
        // 多线程
        //=========================================================
    protected:
        std::mutex mtx;
        bool m_mesh_load_finished[256] = { false };     // Mesh 加载完毕标志位
        bool m_loading_finished = false;
        std::vector<RMesh*> p_meshes_thread;
        // 多线程载入Mesh
        void loadModel();
        void processNode_thread(aiNode* node);
        bool isFinished();

        // 通过 ZResource 继承
        void initTemplate() override;
    };

}

