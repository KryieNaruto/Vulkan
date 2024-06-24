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
			return apply(hash_combine<T...>, tp); // ��ҪC++17
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
    // ģ�����ݵĵ�һ�μӹ���Assimp����->std::vector<xxx>,ͬʱ����vkbuffer
    // �������еĶ������ݣ� RMesh��ֻ��¼����
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

        // ����shader ������shader����Material
        virtual RModel* setShader(Shader* _shader = nullptr);
        RMaterial* getRMaterial(uint32_t _mesh_index) { return p_materials[_mesh_index]; }
        const std::vector<RMaterial*>& getRMaterials() { return p_materials; }
        const std::vector<RMesh*>& getRMeshes() { return p_meshes; }
        ThirdParty::VertexBuffer* getVertexBuffer() { return p_vertex_buffer; }

        // ����Ϊ������ĳ�Ա
        void setParent(void* _parent) { p_parent = _parent; }
        void* isMember() { return p_parent; }
        Shader* getShader() { return p_shader; }

        const std::string& getName() { return m_res_name; }
        RMeshProperty* getProperty() { return p_property; }
    protected:
        std::string m_path = "";
        std::string m_directroy = "";
        // �ӹ�assimp����
        virtual void setup();
        // ���ݲ�ͬ��shader vertex input binding ������ͬ��vkbuffer
        virtual void setupVkData();
        virtual void setupVkData(bool _only_vertex_buffer,VERTEX_INPUT_FLAG _input, uint32_t _stride);
    protected:
        const aiScene* p_root_scene = nullptr;
        
        ThirdParty::VertexBuffer* p_vertex_buffer;
		void* p_vertices_in_shader = nullptr;
        
        inline static std::unordered_map<VERTEX_INPUT_FLAG, std::unordered_map<RModel*, ThirdParty::VertexBuffer*>> p_vertex_buffers;
        inline static std::unordered_map<VERTEX_INPUT_FLAG, std::unordered_map<RModel*, void*>> p_vertices_in_shaders;
        
        std::unordered_map<RVertex*, uint32_t, std::hash<RVertex*>, equal> p_unique_vertices;   // ȥ�ض���
        std::vector<RVertex*> p_vertices;
        std::vector<RMesh*> p_meshes;
        std::unordered_map<uint32_t, std::vector<RTexture*>> p_textures;
        // �ϲ�ȥ�غ��Material
        std::unordered_map<uint32_t, std::vector<RTexture*>> p_merge_textures;
        // p_texתp_merge_tex, [MaterialIndex] [MergeMaterialIndex]
        std::unordered_map<uint32_t, uint32_t> p_texture_To_merge_textures;
        // ����
        Shader* p_shader = nullptr;
        std::vector<RMaterial*> p_materials; // �������ʵĸ���
        RMeshProperty* p_property = nullptr;
        /// ��������
        void init_0();  // ��ʼ��һЩ����Ķ���
        void processAllVertex();        // ��ȡscene�е�����vertex
        void processAllMaterial();      // ��scene�е���������ȫ����������
        void processNode(aiNode* node); // ����node
        void mergeAllMaterial();        // �ϲ���ͬ����
        void loadFromFile(const std::string& _path);
        void loadFromModel(const RModel* _p);
        std::vector<RTexture*> loadMaterial(aiMaterial* mat, aiTextureType _type);

        // �Ƿ���������ĳ�Ա
        void* p_parent = nullptr;
        // �Ƿ��Ǵ�����Model�����
        bool m_is_copy = false;
        //=========================================================
        // ���߳�
        //=========================================================
    protected:
        std::mutex mtx;
        bool m_mesh_load_finished[256] = { false };     // Mesh ������ϱ�־λ
        bool m_loading_finished = false;
        std::vector<RMesh*> p_meshes_thread;
        // ���߳�����Mesh
        void loadModel();
        void processNode_thread(aiNode* node);
        bool isFinished();

        // ͨ�� ZResource �̳�
        void initTemplate() override;
    };

}

