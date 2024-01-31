#pragma once

#include "CgModelSurface.h"

using CgModelPtr = std::shared_ptr<struct CgModel>;

struct CgModel
{
    static CgModelPtr create() { return std::make_shared<CgModel>(); }

    MatrixXf V;   // vertices
    MatrixXf N;   // vertex normals
    MatrixXf FN;  // face normals
    MatrixXf UV0; // uv0
    MatrixXf UV1; // uv1

    // list of Surfaces
    std::vector<CgModelSurface> S;
    std::vector<Texture> textures;
    std::vector<Image> images;
    std::vector<Sampler> samplers;

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
        UV0.resize (2, 0);
        UV1.resize (2, 0);
        triCount = 0;
        S.clear();
    }

    bool isValid()
    {
        if (V.cols() < 3) return false;
        if (N.cols() > 0 && V.cols() != N.cols()) return false;
        if (triangleCount() == 0) return false;
        if (S.size() == 0) return false;

        return true;
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