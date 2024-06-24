#pragma once
#include "Shape.h"

namespace Core::Resource {

#define SHAPE_PLANE "shape-plane"

    class Plane :
        public Shape
    {
	public:
		Plane();
		Plane(const std::string& _name);
		virtual ~Plane();
	protected:
		void generateModel(uint32_t _quad) override;
    };

}
