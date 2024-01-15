#pragma once

#include "cgModelSurface.h"

using cgModelPtr = std::shared_ptr<struct cgModel>;

struct cgModel
{
    static cgModelPtr create() { return std::make_shared<cgModel>(); }

    MatrixXf V;  // vertices
    MatrixXf N;  // vertex normals
    MatrixXf FN; // face normals
    MatrixXf UV; // uvs

    // list of Surfaces
    std::vector<cgModelSurface> S;

    size_t triCount = 0; // must be computed
    size_t vertexCount() const { return V.cols(); }
    size_t triangleCount()
    {
        // compute total face count if neccessary
        if (triCount == 0 && S.size())
        {
            MatrixXu allIndices;
            getAllSurfaceIndices (allIndices);
        }
        return triCount;
    }

    void reset()
    {
        V.resize (3, 0);
        N.resize (3, 0);
        UV.resize (2, 0);
        triCount = 0;
        S.clear();
    }

    bool isValid()
    {
        // must have at least 1 triangle and
        // the vertex and normals counts must match
        return V.cols() == N.cols() && triangleCount() > 0;
    }

    void transformVertices (const Eigen::Affine3f& t)
    {
        for (int i = 0; i < V.cols(); ++i)
        {
            Eigen::Vector3f p = V.col (i);
            V.col (i) = t * p;
        }
    }

    void getAllSurfaceIndices (MatrixXu& allIndices, bool unwelded = false)
    {
        triCount = 0;
        for (const auto& s : S)
        {
            triCount += s.triangleCount();
        }

        allIndices.resize (3, triCount);

        int index = 0;
        for (const auto& s : S)
        {
            size_t triCount = s.triangleCount();

            for (int i = 0; i < triCount; i++)
                allIndices.col (index++) = s.indices().col (i);
        }
    }
};