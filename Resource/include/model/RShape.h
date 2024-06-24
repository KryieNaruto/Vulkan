#pragma once
#include "RModel.h"
namespace Core::Resource {
    
    // 基本形状，诸如球体，立方体,只包含一个RMesh
    // 顶点生成
    // 弃用！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
    class ENGINE_API_RESOURCE RShape :
        public RModel
    {
    public:
        RShape() :RModel() { g_shape_count++; }
        RShape(const std::string& _name);
        virtual ~RShape();
        inline static std::unordered_map<std::string, std::vector<RVertex*>> g_vertices;
        inline static std::unordered_map<std::string, std::vector<uint32_t>> g_indices;
		inline static std::unordered_map<std::string, std::vector<RVertex*>>& getVertices() {return g_vertices;}
		inline static std::unordered_map<std::string, std::vector<uint32_t>>& getIndices() {return g_indices;}
        static void saveVertex(const std::string& _type, const std::vector<RVertex*>& _v);
        static void saveIndex(const std::string& _type, const std::vector<uint32_t>& _v);
		static const std::vector<RVertex*>& getVertex(const std::string& _type);
		static const std::vector<uint32_t>& getIndex(const std::string& _type);
    protected:
        std::string m_type = "RShape";

        // 生成顶点
        virtual void generateModel(uint32_t _quad);
        // 生成切线, 输入索引，三个一组
        void generateTangentAndBitangent(int _indices[3]);
        // 生成切线, 输入索引起点,索引数组
        void generateTangentAndBitangent(const std::vector<uint32_t>& _indices,int _indices_start);
        // 计算切线
        const std::vector<glm::vec3>& calculateTangentAndBitangent(const glm::vec3 _pos[3], const glm::vec2 _uv[3]);
        // 边顶点数
        uint32_t m_quad = 2;

        // 总数量
        inline static uint32_t g_shape_count = 0;

        virtual void setup();
    };
}

