#include "CgMeshOps.h"

using Eigen::Affine3f;
using Eigen::AngleAxisf;
using Eigen::Vector3f;

void CgMeshOps::centerVertices (CgModelPtr model, float scale)
{
    AlignedBox3f modelBound;
    modelBound.min() = model->V.rowwise().minCoeff();
    modelBound.max() = model->V.rowwise().maxCoeff();

    int pointCount = model->V.cols();
    Vector3f center = modelBound.center();
    for (int i = 0; i < pointCount; i++)
    {
        Vector3f pnt = model->V.col (i);
        pnt -= center;
        pnt *= scale;
        model->V.col (i) = pnt;
    }
}

// makes the largest dimension equal to 1
void CgMeshOps::normalizeSize (CgModelPtr model, float& scale)
{
    AlignedBox3f modelBound;
    modelBound.min() = model->V.rowwise().minCoeff();
    modelBound.max() = model->V.rowwise().maxCoeff();

    Vector3f edges = modelBound.max() - modelBound.min();
    float maxEdge = std::max (edges.x(), std::max (edges.y(), edges.z()));
    scale = 1.0f / maxEdge; // max
}

void CgMeshOps::rotateModel (CgModelPtr model, float angleInDegrees, const Vector3f& axisOfRotation)
{
    Affine3f t = Affine3f::Identity();
    float angleInRadians = angleInDegrees * M_PI / 180.0f;

    // Create an AngleAxis object representing the rotation
    AngleAxisf rotation (angleInRadians, axisOfRotation);

    // Apply the rotation to the affine transformation
    t = t * rotation;

    model->transformVertices (t);
}
