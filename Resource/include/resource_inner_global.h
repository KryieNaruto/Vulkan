#pragma once
#include <ThirdPartyManager/third_party_manager_global.h>
//数学库，包含vector和matrix等，用于声明顶点数据、3D MVP变换等
#define GLM_FORCE_RADIANS
//使GLM的类型满足内存对齐
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//使用Vulkan的深度范围0.0到1.0
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
	// 计算方式 (location+1) * vec[x]
	// 此处添加新的着色器输入类型，于此同时，需要 添加下面的VertexInput_[XXXX] XXXX = 类型名称
	// 然后再RVertex中进行偏移设置，完成注册。
	// 此处非程序生成，历史遗留问题，有时间再完善
	// 2024年5月30日15:32:50
	enum VERTEX_INPUT_FLAG {
		_vec2_1 = 0x02,			// 1 * 2
		_vec3_1 = 0x03,			// 1 * 3
		_vec3_1_vec2_2 = 0x07,	// 1 * 3 + 2 * 2
		_default = 0x0F,		// 1 * 3 + 2 * 3 + 3 * 2
		_default_T = 0x1B,	// 1 * 3 + 2 * 3 + 3 * 2 + 4 * 3	// 切线, 副切线通过 格拉姆-施密特正交化过程（Gram-Schmidt process）求得
		_default_T_B = 0x2A,	// 1 * 3 + 2 * 3 + 3 * 2 + 4 * 3 + 5 * 3	// 切线 副切线
	};
	// 光源类型
	enum ENGINE_API_RESOURCE LIGHT_TYPE {
		AMBIENT = 0x00,	// 环境光
		POINT = 0x10,	// 点光源
		PARALLEL = 0x11,	// 平行光
		SPOT = 0x12,	// 聚光灯
	};

	ENGINE_API_RESOURCE LIGHT_TYPE getLightType(uint32_t _index);
	ENGINE_API_RESOURCE uint32_t getLightTypeIndex(LIGHT_TYPE _type);

	// 传入着色器的Layout基本属性
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

	// RObject 基本属性
	// Front 默认为指向世界Z轴。
	typedef struct ENGINE_API_RESOURCE RObjectProperty {
		RObjectProperty();
		virtual ~RObjectProperty();
		// 更新仅包含Model矩阵，坐标轴，Input移动和旋转的响应。
		// 旋转作用在四元数的旋转轴上，视觉响应为绕物体原点旋转，如需绕点旋转，需重写update覆盖。
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
		// 物体坐标系
		glm::vec3 m_X = glm::vec3(1, 0, 0);
		glm::vec3 m_Y = glm::vec3(0, 1, 0);
		glm::vec3 m_Z = glm::vec3(0, 0, 1);
		glm::vec3 m_right	= m_X;
		glm::vec3 m_up		= m_Y;
		glm::vec3 m_front	= m_Z;
		// 欧拉角旋转。用于UI显示
		glm::vec3 m_rotate_euler_world_degree   = glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_euler_world_radians  = glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_euler_object_degree  = glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_euler_object_radians = glm::vec3(0, 0, 0);
		// 四元数旋转，用于内部计算
		// 旋转四元数旋转轴
		glm::vec3 m_rotate_axis_degree	= glm::vec3(0, 0, 0);
		glm::vec3 m_rotate_axis_radians	= glm::vec3(0, 0, 0);
		glm::quat m_rotate_quat_world_x = glm::quat(1, 1, 0, 0);
		glm::quat m_rotate_quat_world_y = glm::quat(1, 0, 1, 0);
		glm::quat m_rotate_quat_world_z = glm::quat(1, 0, 0, 1);
		// XYZ 旋转轴 / cos(theta/2)，W = cos(theta / 2), 单位四元数[0,0,0,1]
		// glm::quat(w,x,y,z)
		glm::quat m_rotate_quat_world = glm::quat(1,0,0,0);
		glm::quat m_rotate_quat_object = glm::quat(1, 0, 0, 0);
		// 缩放
		glm::vec3 m_scale = glm::vec3(1, 1, 1);
		// 模型矩阵
		glm::mat4 m_model = glm::mat4(1);
		glm::mat4 m_model_inverse	= glm::mat4(1);
		// 用于LightSpaceMatrix
		bool m_is_camera = false;
		glm::mat4 m_mat4_view_proj	= glm::mat4(1);
		glm::mat4 m_mat4_view_ortho	= glm::mat4(1);
		// 用于与Input互相响应的变量
		double m_time_current	= 0.0f;
		double m_time_last		= 0.0f;
		float  m_wheel			= 0.0f;				// 滚轮
		glm::vec3 m_move		= glm::vec3(0);		// 移动量
		glm::vec3 m_axis_rotate = glm::vec3(0);		// 旋转轴偏移量

		// 记录是否改变
		glm::mat4 m_model_last = glm::mat4(0);
		// 正交投影范围
		float m_parallel_R		= 10.0f;			// 当光照类型为平行光，视觉显示距离以及深度图范围
		// 标志位
		bool m_is_change		= false;		// 物体是否改变
		bool m_input			= false;		// 是否响应输入
		bool m_hidden			= false;		// 是否隐藏

		inline static ImGuiContext* g_imgui_context = nullptr;
		ThirdParty::imgui::Template* p_template = nullptr;
	}ObjectProperty;

	struct ENGINE_API_RESOURCE SceneObjectProperty :public RObjectProperty {
		void initTemplate() override;
		SceneObjectProperty() { m_name = "SceneObject Property"; }
	};

	struct ENGINE_API_RESOURCE RLightProperty :public SceneObjectProperty
	{
		// 如果不设置m_rotate_euler_world_degree, 默认向下，即-90.0f;
		RLightProperty();
		void update();
		glm::vec3 m_color				= glm::vec3(1);     // 颜色
		glm::vec3 m_attenuation			= glm::vec3(1.0f, 0.08f, 0.032f);	// 光照衰减
		float m_strength				= 1.0f;				// 强度
		float m_radians					= 1000.0f;			// 光源范围, 如果超出范围，则不参与光照计算
		float m_cut_off_deg				= 90.0f;			// 聚光范围，全角
		float m_cut_off_outer_deg		= 100.0f;			// 聚光外圈范围，全角
		float m_cut_off					= glm::cos(glm::radians(m_cut_off_deg));
		float m_cut_off_outer			= glm::cos(glm::radians(m_cut_off_outer_deg));
		float m_radiance				= 300.0f;			// 辐射量
		LIGHT_TYPE m_type				= POINT;			// 类型
		LIGHT_TYPE m_type_last			= POINT;			// 类型
		void initTemplate() override;
	};

	// RMesh 基本属性
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

	// Camera 的基本属性
	/* 引擎使用左手坐标系                                                   */
	/* 向右正方向：X ，向上正方向：Y，屏幕向里：Z                           */
	/* VK的NDC坐标系是在左手坐标系的基础上，Y轴翻转                         */
	struct ENGINE_API_RESOURCE CameraProperty
		:public SceneObjectProperty
	{
		CameraProperty() { m_name = "Camera Property"; }

		void initTemplate() override;
		// 操控方式
		enum CONTROLLER_TYPE {
			ROTATION_AROUND_POINT,		// 绕点旋转
			FIRST_PERSON_VIEW,			// 第一人称
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
		float m_aspect		= 1.0f;		// 画布宽高，非窗口宽高
		float m_zNear		= 0.1f;
		float m_zFar		= 100.0f;
		float m_fov_deg		= 45.0f;
		float m_fov_rad		= glm::radians(m_fov_deg);
		float m_move_speed	= 1.0f;		// 灵敏度
		float m_rotate_R	= 1.0f;		// 旋转半径

		glm::vec3 m_vec3_target				= glm::vec3(0);
		glm::mat4 m_mat4_view				= glm::mat4(1);
		glm::mat4 m_mat4_proj				= glm::mat4(1);
		glm::mat4 m_mat4_proj_inverse		= glm::mat4(1);
		//glm::mat4 m_mat4_view_proj			= glm::mat4(1);	// 转移到RObjectProerty
		glm::mat4 m_mat4_view_inverse		= glm::mat4(1);
		glm::mat4 m_mat4_view_proj_inverse	= glm::mat4(1);
		glm::mat4 m_mat4_ortho				= glm::mat4(1);
		glm::mat4 m_mat4_ortho_inverse		= glm::mat4(1);

		glm::vec2		m_window_pos		= { 0,0 };
		// 屏幕窗口大小(显示画布)
		glm::vec2		m_window_size		= { 0,0 };
		// 画布大小
		glm::vec2		m_canvas_vec2		= { 0,0 };
		VkExtent2D		m_canvas_2d			= { 0,0 };
		VkExtent3D		m_canvas_3d			= { 0,0,0 };
		VkClearValue	m_canvas_bg			= { 0,0,0,1 };
		VkClearValue	m_canvas_dp			= { 1.0f, 0 };
		// 基于该窗口的鼠标位置
		glm::vec2		m_mouse_pos_relative		= { 0,0 };
		glm::vec2		m_last_mouse_pos_relative	= { 0,0 };
		// 正交投影width height
		float m_project_width = 0.0f;
		float m_project_height = 0.0f;


		static float& getDepthBiasConstant() { return m_depthBiasConstant; }
		static float& getDepthBiasSlop() { return m_depthBiasSlope; }
		// 深度偏移
		// 深度偏移 (aka "Polygon offset")
		// 用来避免 shadow mapping artifacts
		// 深度偏移值常量
		inline static float m_depthBiasConstant = 1.25f;
		// 深度斜率的偏移值，会根据几何的斜率做偏移
		inline static float m_depthBiasSlope = 0.005f;
	};
}