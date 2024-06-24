#pragma once
#include "ZResource.h"
#include "resource_inner_global.h"
#include "model/RShape.h"
namespace Core::Resource {
    //----------------------------
    // ��Դ
    // �Գ���Ϊ��λ��Ŀǰû�г���������࣬��ʱ��cameraΪ��λ
    // 2024��04��05�� 20:02:04
    // ȥ����RLight�Ļ��ƹ��ܣ�ת�Ƶ�Light����
    //----------------------------
    class ENGINE_API_RESOURCE RLight :
        public ZResource
    {
    public:
        RLight();
        RLight(const std::string& _name);
        RLight(LIGHT_TYPE _type, glm::vec3 _pos = glm::vec3(0), glm::vec4 _color = glm::vec4(1), glm::vec3 _dir = glm::vec3(0));
		virtual ~RLight();
		virtual Json::Value serializeToJSON();
		virtual void deserializeToObj(Json::Value& root);
    protected:
        RLightProperty* p_light_property;

        void init_0();
    public:
        RLight* update();
        std::string getName() { return m_res_name; }
        RLightProperty* getProperty() { return p_light_property; }
        RLight* setPos(glm::vec3 _pos) {
            if (p_light_property) {
                p_light_property->m_vec3_pos = _pos;
            }
            return this;
        }
        RLight* setType(LIGHT_TYPE _type) {
            if (p_light_property) {
                p_light_property->m_type = _type;
            }
            return this;
        }
		RLight* setColor(glm::vec3 _color, float _strength = 1.0f) {
			if (p_light_property) {
				p_light_property->m_color = _color;
				p_light_property->m_strength = _strength;
			}
            return this;
		}
        RLight* setDirection(glm::vec3 _dir, LIGHT_TYPE _type) {
            if (p_light_property) {
                p_light_property->m_rotate_euler_world_degree = _dir;
                p_light_property->m_type = _type;
            }
            return this;
        }


        // ͨ�� ZResource �̳�
        void initTemplate() override;

    };

}

