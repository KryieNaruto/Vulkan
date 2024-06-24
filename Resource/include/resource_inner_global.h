#pragma once
#include <ThirdPartyManager/third_party_manager_global.h>
//��ѧ�⣬����vector��matrix�ȣ����������������ݡ�3D MVP�任��
#define GLM_FORCE_RADIANS
//ʹGLM�����������ڴ����
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//ʹ��Vulkan����ȷ�Χ0.0��1.0
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// Quat WXYZ
#define GLM_FORCE_QUAT_DATA_WXYZ
#include <ThirdParty/glm/glm.hpp>
#include <ThirdParty/glm/gtc/quaternion.hpp>
#include <ThirdParty/glm/gtc/matrix_transform.hpp>
#include <ThirdParty/glm/gtc/type_ptr.hpp>
#include <map>
#include <string>

#ifdef RESOURCE_EXPORTS
#define ENGINE_API_RESOURCE __declspec(dllexport)
#else
#define ENGINE_API_RESOURCE __declspec(dllimport)
#endif

#define RENDER_PASS_NODEPTH "render_pass_color_attachment_r8g8b8unorm_sampler1bit_colorattachment"
#define RENDER_PASS_RG_NODEPTH "render_pass_color_attachment_r8g8unorm_sampler1bit_colorattachment"
#define RENDER_PASS_0 "render_pass_color_attachment_r8g8b8a8unorm_sampler1bit_colorattachment_depth_stencilattachment"
#define RENDER_PASS_1 "render_pass_color_attachment_d32sfloats8uint_sampler1bit_depthstencilattachment"
#define RENDER_PASS_2 "render_pass_depth_attachment"

#define SHADER_2D_MESH "shader_2d_mesh"
#define SHADER_DEFAULT "shader_default"
#define SHADER_LIGHT	"shader_light"

#define SAMPLER "Sampler"
#define SAMPLER_REPEAT "Sampler Repeat"

namespace Core::Resource::Pool {
	void* getRRenderPass(const std::string& _name);
	ThirdParty::Pipeline* getThirdParty_Pipeline(const std::string& _name);

	
	void addRRenderPassIntoPool(const std::string& _name, void* _r);

	void addThirdParty_PipelineInfoPool(const std::string& _name, ThirdParty::Pipeline* _r);
	void addThirdParty_SamplerInfoPool(const std::string& _name, ThirdParty::Sampler* _r);
}
namespace Core::Resource {
	// ���㷽ʽ (location+1) * vec[x]
	// �˴�����µ���ɫ���������ͣ��ڴ�ͬʱ����Ҫ ��������VertexInput_[XXXX] XXXX = ��������
	// Ȼ����RVertex�н���ƫ�����ã����ע�ᡣ
	// �˴��ǳ������ɣ���ʷ�������⣬��ʱ��������
	// 2024��5��30��15:32:50
	enum VERTEX_INPUT_FLAG {
		_vec2_1 = 0x02,			// 1 * 2
		_vec3_1 = 0x03,			// 1 * 3
		_vec3_1_vec2_2 = 0x07,	// 1 * 3 + 2 * 2
		_default = 0x0F,		// 1 * 3 + 2 * 3 + 3 * 2
		_default_T = 0x1B,	// 1 * 3 + 2 * 3 + 3 * 2 + 4 * 3	// ����, ������ͨ�� ����ķ-ʩ�������������̣�Gram-Schmidt process�����
		_default_T_B = 0x2A,	// 1 * 3 + 2 * 3 + 3 * 2 + 4 * 3 + 5 * 3	// ���� ������
	};
	// ��Դ����
	enum ENGINE_API_RESOURCE LIGHT_TYPE {
		AMBIENT = 0x00,	// ������
		POINT = 0x10,	// ���Դ
		PARALLEL = 0x11,	// ƽ�й�
		SPOT = 0x12,	// �۹��
	};

	ENGINE_API_RESOURCE LIGHT_TYPE getLightType(uint32_t _index);
	ENGINE_API_RESOURCE uint32_t getLightTypeIndex(LIGHT_TYPE _type);

	// ������ɫ����Layout��������
	struct VertexInput_vec2_1 {
		glm::vec2 location_0;
	};
	struct VertexInput_vec3_1 {
		glm::vec3 location_0;
	};
	struct VertexInput_default {
		glm::vec3 location_0;	// pos
		glm::vec3 location_1;	// normal
		glm::vec2 location_2;	// taxCoord
	};
	struct VertexInput_default_T
	{
		glm::vec3 location_0;// pos
		glm::vec3 location_1;// normal
		glm::vec2 location_2;// taxCoord
		glm::vec3 location_3;// T
	};
	struct VertexInput_default_T_B
	{
		glm::vec3 location_0;// pos
		glm::vec3 location_1;// normal
		glm::vec2 location_2;// taxCoord
		glm::vec3 location_3;// T
		glm::vec3 location_4;// B
	};
	struct VertexInput_vec3_1_vec2_2 {
		glm::vec3 location_0;	// pos
		glm::vec2 location_1;	// taxCoord
	};

	// RObject ��������
	// Front Ĭ��Ϊָ������Z�ᡣ
	typedef struct ENGINE_API_RESOURCE RObjectProperty {
		RObjectProperty();
		virtual ~RObjectProperty();
		// ���½�����Model���������ᣬInput�ƶ�����ת����Ӧ��
		// ��ת��������Ԫ������ת���ϣ��Ӿ���ӦΪ������ԭ����ת�������Ƶ���ת������дupdate���ǡ�
		virtual void update();
		virtual void update(RObjectProperty* _parent);
		virtual void initTemplate();
		virtual RObjectProperty* getParentProperty();
		virtual RObjectProperty* setParentProperty(RObjectProperty* _property);
		RObjectProperty* p_parent_object = nullptr;

		std::string m_name = "RObjectProperty";
		glm::vec3 m_world_X = glm::vec3(1, 0, 0);
		glm::vec3 m_world_Y = glm::vec3(0, 1, 0);
		glm::vec3 m_world_Z = glm::vec3(0, 0, 1);
		glm::vec3 m_vec3_pos = glm::vec3(0);
		// ��������ϵ
		glm::vec3 m_X = glm::vec3(1, 0, 0);
		glm::vec3 m_Y = glm::vec3(0, 1, 0);
		glm::vec3 m_Z = glm::vec3(0, 0, 1);
		glm::vec3 m_right	= m_X;
		glm::vec3 m_up		= m_Y;
		glm::vec3 m_front	= m_Z;
		// ŷ������ת������UI��ʾ
		glm::vec3 m_rotate_euler_world_degree   = glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_euler_world_radians  = glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_euler_object_degree  = glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_euler_object_radians = glm::vec3(0, 0, 0);
		// ��Ԫ����ת�������ڲ�����
		// ��ת��Ԫ����ת��
		glm::vec3 m_rotate_axis_degree	= glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_axis_radians	= glm::vec3(0, 0, 0);
		glm::quat m_rotate_quat_world_x = glm::quat(1, 1, 0, 0);
		glm::quat m_rotate_quat_world_y = glm::quat(1, 0, 1, 0);
		glm::quat m_rotate_quat_world_z = glm::quat(1, 0, 0, 1);
		// XYZ ��ת�� / cos(theta/2)��W = cos(theta / 2), ��λ��Ԫ��[0,0,0,1]
		// glm::quat(w,x,y,z)
		glm::quat m_rotate_quat_world = glm::quat(1,0,0,0);
		glm::quat m_rotate_quat_object = glm::quat(1, 0, 0, 0);
		// ����
		glm::vec3 m_scale = glm::vec3(1, 1, 1);
		// ģ�;���
		glm::mat4 m_model = glm::mat4(1);
		glm::mat4 m_model_inverse	= glm::mat4(1);
		// ����LightSpaceMatrix
		bool m_is_camera = false;
		glm::mat4 m_mat4_view_proj	= glm::mat4(1);
		glm::mat4 m_mat4_view_ortho	= glm::mat4(1);
		// ������Input������Ӧ�ı���
		double m_time_current	= 0.0f;
		double m_time_last		= 0.0f;
		float  m_wheel			= 0.0f;				// ����
		glm::vec3 m_move		= glm::vec3(0);		// �ƶ���
		glm::vec3 m_axis_rotate = glm::vec3(0);		// ��ת��ƫ����

		// ��¼�Ƿ�ı�
		glm::mat4 m_model_last = glm::mat4(0);
		// ����ͶӰ��Χ
		float m_parallel_R		= 10.0f;			// ����������Ϊƽ�й⣬�Ӿ���ʾ�����Լ����ͼ��Χ
		// ��־λ
		bool m_is_change		= false;		// �����Ƿ�ı�
		bool m_input			= false;		// �Ƿ���Ӧ����
		bool m_hidden			= false;		// �Ƿ�����

		inline static ImGuiContext* g_imgui_context = nullptr;
		ThirdParty::imgui::Template* p_template = nullptr;
	}ObjectProperty;

	struct ENGINE_API_RESOURCE SceneObjectProperty :public RObjectProperty {
		void initTemplate() override;
		SceneObjectProperty() { m_name = "SceneObject Property"; }
	};

	struct ENGINE_API_RESOURCE RLightProperty :public SceneObjectProperty
	{
		// ���������m_rotate_euler_world_degree, Ĭ�����£���-90.0f;
		RLightProperty();
		void update();
		glm::vec3 m_color				= glm::vec3(1);     // ��ɫ
		glm::vec3 m_attenuation			= glm::vec3(1.0f, 0.08f, 0.032f);	// ����˥��
		float m_strength				= 1.0f;				// ǿ��
		float m_radians					= 1000.0f;			// ��Դ��Χ, ���������Χ���򲻲�����ռ���
		float m_cut_off_deg				= 90.0f;			// �۹ⷶΧ��ȫ��
		float m_cut_off_outer_deg		= 100.0f;			// �۹���Ȧ��Χ��ȫ��
		float m_cut_off					= glm::cos(glm::radians(m_cut_off_deg));
		float m_cut_off_outer			= glm::cos(glm::radians(m_cut_off_outer_deg));
		float m_radiance				= 300.0f;			// ������
		LIGHT_TYPE m_type				= POINT;			// ����
		LIGHT_TYPE m_type_last			= POINT;			// ����
		void initTemplate() override;
	};

	// RMesh ��������
	struct ENGINE_API_RESOURCE RMeshProperty
		:public SceneObjectProperty
	{
		RMeshProperty() { m_name = "RMesh Property"; }

		void initTemplate() override;
		void update() { RObjectProperty::update(); };
		void update(RMeshProperty* _parent);
		glm::vec3 m_vec3_color			= glm::vec3(0.5);
		int		  m_texture_count		= 0;
		int		  m_shinness			= 16;
		int		  m_has_shadow			= 1;
	};

	// Camera �Ļ�������
	/* ����ʹ����������ϵ                                                   */
	/* ����������X ������������Y����Ļ���Z                           */
	/* VK��NDC����ϵ������������ϵ�Ļ����ϣ�Y�ᷭת                         */
	struct ENGINE_API_RESOURCE CameraProperty
		:public SceneObjectProperty
	{
		CameraProperty() { m_name = "Camera Property"; }

		void initTemplate() override;
		// �ٿط�ʽ
		enum CONTROLLER_TYPE {
			ROTATION_AROUND_POINT,		// �Ƶ���ת
			FIRST_PERSON_VIEW,			// ��һ�˳�
		};
		enum PROJECT_TYPE {
			PERSPECTIVE,
			ORTHO,
		};
		virtual void update();
		virtual void update(ObjectProperty* _obj_prop);

		CONTROLLER_TYPE m_controller_type	= ROTATION_AROUND_POINT;
		PROJECT_TYPE	m_project_type		= PERSPECTIVE;

		int m_light_count	= 0;
		float m_aspect		= 1.0f;		// ������ߣ��Ǵ��ڿ��
		float m_zNear		= 0.1f;
		float m_zFar		= 100.0f;
		float m_fov_deg		= 45.0f;
		float m_fov_rad		= glm::radians(m_fov_deg);
		float m_move_speed	= 1.0f;		// ������
		float m_rotate_R	= 1.0f;		// ��ת�뾶

		glm::vec3 m_vec3_target				= glm::vec3(0);
		glm::mat4 m_mat4_view				= glm::mat4(1);
		glm::mat4 m_mat4_proj				= glm::mat4(1);
		glm::mat4 m_mat4_proj_inverse		= glm::mat4(1);
		//glm::mat4 m_mat4_view_proj			= glm::mat4(1);	// ת�Ƶ�RObjectProerty
		glm::mat4 m_mat4_view_inverse		= glm::mat4(1);
		glm::mat4 m_mat4_view_proj_inverse	= glm::mat4(1);
		glm::mat4 m_mat4_ortho				= glm::mat4(1);
		glm::mat4 m_mat4_ortho_inverse		= glm::mat4(1);

		glm::vec2		m_window_pos		= { 0,0 };
		// ��Ļ���ڴ�С(��ʾ����)
		glm::vec2		m_window_size		= { 0,0 };
		// ������С
		glm::vec2		m_canvas_vec2		= { 0,0 };
		VkExtent2D		m_canvas_2d			= { 0,0 };
		VkExtent3D		m_canvas_3d			= { 0,0,0 };
		VkClearValue	m_canvas_bg			= { 0,0,0,1 };
		VkClearValue	m_canvas_dp			= { 1.0f, 0 };
		// ���ڸô��ڵ����λ��
		glm::vec2		m_mouse_pos_relative		= { 0,0 };
		glm::vec2		m_last_mouse_pos_relative	= { 0,0 };
		// ����ͶӰwidth height
		float m_project_width = 0.0f;
		float m_project_height = 0.0f;


		static float& getDepthBiasConstant() { return m_depthBiasConstant; }
		static float& getDepthBiasSlop() { return m_depthBiasSlope; }
		// ���ƫ��
		// ���ƫ�� (aka "Polygon offset")
		// �������� shadow mapping artifacts
		// ���ƫ��ֵ����
		inline static float m_depthBiasConstant = 1.25f;
		// ���б�ʵ�ƫ��ֵ������ݼ��ε�б����ƫ��
		inline static float m_depthBiasSlope = 0.005f;
	};
}