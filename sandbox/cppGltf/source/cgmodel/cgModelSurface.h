#pragma once

#include "../GLTFUtil.h"

// a Surface is a group of triangles with a unique Material
struct cgModelSurface
{
    std::string name;
    MatrixXu F; // triangle indices
    Material material;

    const size_t triangleCount() const { return F.cols(); }
    MatrixXu& indices() { return F; }
    const MatrixXu& indices() const { return F; }
};