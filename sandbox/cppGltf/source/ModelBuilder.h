#pragma once

#include "GLTFUtil.h"
#include "cgmodel/cgModel.h"

using ModelList = std::vector<cgModelPtr>;

class ModelBuilder
{
 public:
    ModelBuilder (const GLTFData& data) :
        data (data)
    {
    }

    cgModelPtr createCgModel();

 private:
    const GLTFData& data;

    cgModelPtr buildModelList();
    cgModelPtr forgeIntoOne (const ModelList& models);

    void getTriangleIndices (MatrixXu& matrix, const Accessor& accessor);
    void getVertexFloatAttribute (Eigen::MatrixXf& matrix, const Accessor& accessor);
};