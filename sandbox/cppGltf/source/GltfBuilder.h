#pragma once

#include "cgModel/CgModel.h"
#include "GltfUtil.h"

class GltfBuilder
{
 public:
    GltfBuilder();

    void convertCgModelToGltfData (const CgModelPtr& cgModel, GLTFData& gltfData, const fs::path& pathToBinary);

 private:
    void fillMaterials (GLTFData& gltfData, const CgModelPtr& cgModel);

};