#pragma once

#include "GLTFUtil.h"
#include "cgmodel/CgModel.h"

using ModelList = std::vector<CgModelPtr>;

class CgModelBuilder
{
 public:
    CgModelBuilder (const GLTFData& data) :
        data (data)
    {
    }

    CgModelPtr createCgModel();

 private:
    const GLTFData& data;

    CgModelPtr buildModelList();
    CgModelPtr forgeIntoOne (const ModelList& models);

    void getTriangleIndices (MatrixXu& matrix, const Accessor& accessor);
    void getVertexFloatAttribute (MatrixXf& matrix, const Accessor& accessor);
    void applyNodeTransforms (const Node& node, const Eigen::Affine3f& parentTransform, const std::vector<Node>& nodes, std::vector<CgModelPtr>& cgModels);
    bool isRootNode (const Node& node, const std::vector<Node>& nodes);
};