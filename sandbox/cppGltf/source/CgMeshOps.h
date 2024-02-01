#pragma once

#include "cgmodel/CgModel.h"

using Eigen::AlignedBox3f;
using Eigen::Vector3f;

class CgMeshOps
{
 public:
    CgMeshOps() = default;
    ~CgMeshOps() = default;

    void centerVertices (CgModelPtr model, float scale);
    void normalizeSize (CgModelPtr model, float& scale);
    void rotateModel (CgModelPtr model, float angleInDegrees, const Vector3f& axisOfRotation);
};