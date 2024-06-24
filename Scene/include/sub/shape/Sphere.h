#pragma once
#include "Shape.h"
#define SHAPE_SPHERE "shape_sphere"

namespace Core::Resource {

    class Sphere :
        public Shape
    {
    public:
        Sphere();
        Sphere(const std::string& _name);
        virtual ~Sphere();
    protected:
		void generateModel(uint32_t _quad) override;

    };

}

