#pragma once
#include "RShape.h"

namespace Core::Resource {

    //-----------------------------------------
    // 平面
	// 边默认2顶点
    //-----------------------------------------
    class ENGINE_API_RESOURCE RPlane :
        public RShape
    {
	public:
		RPlane();
		RPlane(const std::string& _name);
		virtual ~RPlane();

	protected:
		void generateModel(uint32_t _quad) override;
    };

}

