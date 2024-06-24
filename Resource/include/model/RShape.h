#pragma once
#include "RModel.h"
namespace Core::Resource {
    
    // ������״���������壬������,ֻ����һ��RMesh
    // ��������
    // ���ã�����������������������������������������������������������
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

        // ���ɶ���
        virtual void generateModel(uint32_t _quad);
        // ��������, ��������������һ��
        void generateTangentAndBitangent(int _indices[3]);
        // ��������, �����������,��������
        void generateTangentAndBitangent(const std::vector<uint32_t>& _indices,int _indices_start);
        // ��������
        const std::vector<glm::vec3>& calculateTangentAndBitangent(const glm::vec3 _pos[3], const glm::vec2 _uv[3]);
        // �߶�����
        uint32_t m_quad = 2;

        // ������
        inline static uint32_t g_shape_count = 0;

        virtual void setup();
    };
}

