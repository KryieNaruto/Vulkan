#pragma once
#include <Resource/resource_global.h>
#include "Shape.h"

#define SHAPE_CUBE "shape_cube"

namespace Core::Resource {

    class Cube :
        public Shape
    {
    public:
        Cube();
        Cube(const std::string& _name);
        virtual ~Cube();
    protected:
        void generateModel(uint32_t _quad) override;
    };
}

