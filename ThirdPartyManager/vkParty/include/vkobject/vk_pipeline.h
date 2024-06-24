#pragma once
#include "vk_object.h"

namespace ThirdParty {

    /**
     * VkPipeline 封装，包含Graphics Pipeline创建与销毁功能
     * <Node 属性>
     * * 3 Inputs
     *		Shader Stage: const std::vector<VkPipelineShaderStageCreateInfo>
     *		Render Pass	: VkRenderPass
     *		Pipeline Layout : VkPipelineLayout
     * * 1 Outputs
     *		Pipeline : VkPipeline
     */
    class ENGINE_API_THIRDPARTY Pipeline :
        public vkObject
    {
    public:
        Pipeline(const std::string& _label);
        virtual ~Pipeline();
		inline VkPipeline& getVkPipeline() { return m_pipeline; }

		//------------------顶点输入------------------
		Pipeline* addVertexInputBinding(const VkVertexInputBindingDescription& _binding, const VkVertexInputAttributeDescription& _attribute);
		/**
		* VkVertexInputBindingDescription
		* * * uint32_t             binding;
		* * * uint32_t             stride;
		* * * VkVertexInputRate    inputRate; 
		* VkVertexInputAttributeDescription
		* * * uint32_t    location;
		* * * uint32_t    binding;
		* * * VkFormat    format;
		* * * uint32_t    offset;
		*/
		Pipeline* setVertexInputBinding(
			const std::vector<VkVertexInputBindingDescription>& _bindings		= {}, 
			const std::vector<VkVertexInputAttributeDescription>& _attributes	= {});
		//------------------输入装配------------------
		Pipeline* setInputAssembly(
			VkPrimitiveTopology _topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 
			VkBool32			_primitiveRestartEnable = VK_FALSE);
		//------------------视口裁剪------------------
		/** 限制单视口
		* VkViewport
		* * * float    x;
		* * * float    y;
		* * * float    width;
		* * * float    height;
		* * * float    minDepth;
		* * * float    maxDepth;
		* VkRect2D
		* * * VkOffset2D    offset;
		* * * VkExtent2D    extent;
		*/
		Pipeline* setViewportAndScissor(const VkViewport& _viewport, const VkRect2D& _scissor);
		//-------------------光栅化-------------------
		Pipeline* setRasterization(
			VkPolygonMode               _polygonMode			= VK_POLYGON_MODE_FILL,
			VkCullModeFlags				_cullMode				= VK_CULL_MODE_BACK_BIT,
			VkFrontFace					_frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE,
			float						_lineWidth				= 1.0f,
			VkBool32					_depthClampEnable		= VK_FALSE,
			VkBool32					_rasterizerDisardEnable = VK_FALSE,
			VkBool32					_depthBiasEnable		= VK_FALSE,
			const std::vector<float>&	_depthBias				= {0.0f,0.0f,0.0f});
		//------------------多重采样------------------
		Pipeline* setMultisample(
			VkSampleCountFlags	_sampleCount			= VK_SAMPLE_COUNT_1_BIT, 
			VkBool32			_minSampleShading		= VK_FALSE,
			float				_minSampleShadading		= 1.0f,
			VkSampleMask*		_pSampleMask			= nullptr,
			VkBool32			_alphaToCoverageEnable	= VK_FALSE,
			VkBool32			_alphaToOneEable		= VK_FALSE);
		//---------------深度和模板测试---------------
		Pipeline* setDepthStencil(bool _depth_test = true, bool _depth_write = true,VkCompareOp _cmp_op = VK_COMPARE_OP_LESS_OR_EQUAL);
		//------------------颜色混合------------------
		Pipeline* setColorBlend(
			const std::vector<VkPipelineColorBlendAttachmentState>&
			_color_blends = { {/*
					VkBool32                 blendEnable;
					VkBlendFactor            srcColorBlendFactor;
					VkBlendFactor            dstColorBlendFactor;
					VkBlendOp                colorBlendOp;
					VkBlendFactor            srcAlphaBlendFactor;
					VkBlendFactor            dstAlphaBlendFactor;
					VkBlendOp                alphaBlendOp;
					VkColorComponentFlags    colorWriteMask;
				*/
				VK_TRUE,
				VK_BLEND_FACTOR_SRC_ALPHA ,VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ,VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_ONE,VK_BLEND_FACTOR_ZERO,VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT} },
			VkBool32 _logicOpEnable = VK_FALSE,
			VkLogicOp _logicOp = VK_LOGIC_OP_COPY,const std::vector<float> _blendConstants = {0,0,0,0});
		//--------------------细分--------------------
		Pipeline* setTessellation(VkPipelineTessellationStateCreateInfo& _state);
		//--------------------动态--------------------
		Pipeline* setDynamic(const std::vector<VkDynamicState>& _dynamics);
		//------------------管线布局------------------
		Pipeline* setPipelineLayout(VkPipelineLayout _pipeline_layout);
		//------------------渲染流程------------------
		Pipeline* setRenderPass(VkRenderPass _renderpass,uint32_t _subpass_index);
		//------------------着色器阶段------------------
		Pipeline* setShaderStages(const std::vector<VkPipelineShaderStageCreateInfo>& _shader_stages);

		//------------------创建管线------------------
		bool createGraphicsPipeline();
    private:
		enum PIPELINE_CREATE_STAGE {
			VERTEX_INPUT	= 0x00,
			INPUT_ASSEMBLY	= 0x01,
			VIEWPORT		= 0x02,
			RASTERIZATION	= 0x03,
			MULTISAMPLE		= 0x04,
			DEPTH_STENCIL	= 0x05,
			COLOR_BLEND		= 0x06,
			DYNAMIC			= 0x07,
			PIPELINE_LAYOUT = 0x08,
			RENDERPASS		= 0x09,
			SHADER_STAGE	= 0x09,
			FINISH,
		};
		bool m_flags[PIPELINE_CREATE_STAGE::FINISH] = { false };
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkGraphicsPipelineCreateInfo* p_create_info = nullptr;

		uint32_t m_subpass_index;
		//---------------顶点输入---------------
		std::vector<VkVertexInputBindingDescription> m_bindings;
		std::vector<VkVertexInputAttributeDescription> m_attributes;
		VkPipelineVertexInputStateCreateInfo* p_vertex_input_state_info;
		//---------------输入装配---------------
		VkPipelineInputAssemblyStateCreateInfo* p_input_assembly_state_info;
		//---------------视口裁剪---------------
		std::vector<VkViewport> m_viewports;
		std::vector<VkRect2D> m_scissors;
		VkPipelineViewportStateCreateInfo* p_viewport_state_info = NULL;
		//---------------光栅化---------------
		VkPipelineRasterizationStateCreateInfo* p_raster_state_info = NULL;
		//---------------多重采样---------------
		VkPipelineMultisampleStateCreateInfo* p_multisample_satae_info = NULL;
		//---------------细分着色器---------------
		VkPipelineTessellationStateCreateInfo* p_tessellation_state_info = NULL;
		//---------------深度和模板测试---------------
		VkPipelineDepthStencilStateCreateInfo* p_depth_stencil_state_info = NULL;
		//---------------颜色混合---------------
		std::vector<VkPipelineColorBlendAttachmentState> m_color_blends;
		VkPipelineColorBlendStateCreateInfo* p_color_blend_state_info = NULL;
		//---------------动态---------------
		std::vector<VkDynamicState> m_dynamics;
		VkPipelineDynamicStateCreateInfo* p_dynamic_state_info = NULL;
		//---------------管线布局---------------
		VkPipelineLayout m_pipeline_layout = NULL;
		//------------------渲染流程------------------
		VkRenderPass m_renderpass = NULL;
		//------------------着色器阶段------------------
		std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
	};

}

