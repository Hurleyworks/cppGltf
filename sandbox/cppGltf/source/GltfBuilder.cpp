#include "GltfBuilder.h"
#include <BinaryWriter.h>

using Eigen::Vector3f;

// Constructor
GltfBuilder::GltfBuilder()
{
}

// Function to convert CG Model to GLTF data using CgModelPtr
void GltfBuilder::convertCgModelToGltfData (const CgModelPtr& cgModel, GLTFData& gltfData, const fs::path& pathToBinary)
{
    if (!cgModel->isValid())
        throw std::runtime_error ("Invalid cgModel");

    fillMaterials (gltfData, cgModel);

    BinaryWriter binaryWriter (pathToBinary.generic_string(), true);

    std::vector<BufferView> bufferViews;
    std::vector<Accessor> accessors;
    size_t byteOffset = 0;

    // Write vertex data
    size_t vertexByteLength = cgModel->V.size() * sizeof (float);
    binaryWriter.WriteFromMemory (cgModel->V.data(), vertexByteLength);
    bufferViews.push_back (BufferView{0, byteOffset, vertexByteLength, sizeof (Eigen::Vector3f), 34962}); // Target is 0 for non-GPU buffer
    accessors.push_back (Accessor{static_cast<int> (bufferViews.size() - 1), 0, GLTFComponentType::FLOAT, cgModel->vertexCount(), GLTFAccessorType::VEC3, false});

    int vertexAccessorIndex = 0;
    int normalAccessorIndex = INVALID_INDEX;
    int texCoord0AccessorIndex = INVALID_INDEX;
    int tangentAccessorIndex = INVALID_INDEX;

    // store the min/max bounding box values
    Accessor& acc = accessors.back();
    Vector3f min = cgModel->V.rowwise().minCoeff();
    Vector3f max = cgModel->V.rowwise().maxCoeff();
    acc.minValues.resize (3);
    acc.maxValues.resize (3);
    std::memcpy (acc.minValues.data(), min.data(), 3 * sizeof (float));
    std::memcpy (acc.maxValues.data(), max.data(), 3 * sizeof (float));

    byteOffset += vertexByteLength;

    // Maybe write normal data
    if (cgModel->N.size() > 0)
    {
        size_t normalByteLength = cgModel->N.size() * sizeof (float);
        binaryWriter.WriteFromMemory (cgModel->N.data(), normalByteLength);
        bufferViews.push_back (BufferView{0, byteOffset, normalByteLength, sizeof (Eigen::Vector3f), 34962});
        accessors.push_back (Accessor{static_cast<int> (bufferViews.size() - 1), 0, GLTFComponentType::FLOAT, cgModel->vertexCount(), GLTFAccessorType::VEC3, false});
        byteOffset += normalByteLength;

        normalAccessorIndex = accessors.size() - 1;
    }

    // Write UV data (if present)
    if (cgModel->UV0.size() > 0)
    {
        size_t uvByteLength = cgModel->UV0.size() * sizeof (float);
        binaryWriter.WriteFromMemory (cgModel->UV0.data(), uvByteLength);
        bufferViews.push_back (BufferView{0, byteOffset, uvByteLength, sizeof (Eigen::Vector2f), 34962});
        accessors.push_back (Accessor{static_cast<int> (bufferViews.size() - 1), 0, GLTFComponentType::FLOAT, cgModel->vertexCount(), GLTFAccessorType::VEC2, false});
        byteOffset += uvByteLength;

        texCoord0AccessorIndex = accessors.size() - 1;
    }

    Mesh mesh;

    // Process and write data for each surface as a separate MeshPrimitive
    uint32_t index = 0;
    for (const auto& surface : cgModel->S)
    {
        MeshPrimitive primitive;

        // Write indices for the current surface
        const MatrixXu& indices = surface.indices();
        size_t indexByteLength = indices.size() * sizeof (unsigned int);
        binaryWriter.WriteFromMemory (indices.data(), indexByteLength);

        BufferView indexBufferView{0, byteOffset, indexByteLength, sizeof (unsigned int), 34963};
        bufferViews.push_back (indexBufferView);

        Accessor indexAccessor{static_cast<int> (bufferViews.size() - 1), 0, GLTFComponentType::UNSIGNED_INT, surface.triangleCount() * 3, GLTFAccessorType::SCALAR, false};
        accessors.push_back (indexAccessor);

        // Set the indices for the primitive to the index of the accessor
        primitive.indices = static_cast<int> (accessors.size() - 1);
        primitive.mode = GLTFMeshMode::TRIANGLES;
        primitive.attributes["POSITION"] = vertexAccessorIndex;
        if (normalAccessorIndex != INVALID_INDEX)
        {
            primitive.attributes["NORMAL"] = normalAccessorIndex;
        }

        if (texCoord0AccessorIndex != INVALID_INDEX)
        {
            primitive.attributes["TEXCOORD_0"] = texCoord0AccessorIndex;
        }

        if (index < gltfData.materials.size())
        {
            primitive.material = index;
        }

        mesh.primitives.push_back (primitive);

        byteOffset += indexByteLength;
        ++index;
    }

    // Finalize the buffer
    Buffer buffer;
    buffer.uri = pathToBinary.filename().string();
    ;
    buffer.byteLength = byteOffset;

    // Update GLTFData
    gltfData.buffers.push_back (buffer);
    gltfData.bufferViews = bufferViews;
    gltfData.accessors = accessors;
    gltfData.meshes.push_back (mesh); // Add the mesh to GLTFData
    gltfData.asset = Asset{};
    Node node;
    node.mesh = 0;
    gltfData.nodes.push_back (node);
    Scene scene;
    scene.nodeIndices.push_back (0);
    gltfData.scenes.push_back (scene);
    binaryWriter.Flush(); // Flush the writer to ensure all data is written

    // Print bufferView information
    LOG (INFO) << "Total bufferViews: " << gltfData.bufferViews.size();
    for (const auto& bufferView : gltfData.bufferViews)
    {
        LOG (INFO) << "  BufferView - Buffer: " << bufferView.bufferIndex;
        LOG (INFO) << "  Byte Length: " << bufferView.byteLength;
        LOG (INFO) << "  Byte Offset: " << bufferView.byteOffset;
        LOG (INFO) << "  Stride: " << bufferView.byteStride;

        LOG (INFO) << "_____________";
        // ... other bufferView details ...
    }

    LOG (INFO) << "Total accessors: " << gltfData.accessors.size();
    for (const auto& accessor : gltfData.accessors)
    {
        LOG (INFO) << "  Accessor Component Type: " << componentTypeToString (accessor.componentType);
        LOG (INFO) << "  Accessor BufferView index: " << accessor.bufferViewIndex;
        LOG (INFO) << "  Accessor ByteOffset: " << accessor.byteOffset;
        LOG (INFO) << "  Accessor Count: " << accessor.count;
        LOG (INFO) << "  Accessor Type: " << accessorTypeToString (accessor.type);

        LOG (INFO) << "_____________";
        // ... other accessor details ...
    } // Print bufferView information
}

void GltfBuilder::fillMaterials (GLTFData& gltfData, const CgModelPtr& cgModel)
{
    if (!cgModel || !cgModel->isValid())
    {
        throw std::runtime_error ("Invalid cgModel");
    }

    gltfData.images = cgModel->images;
    gltfData.textures = cgModel->textures;
    gltfData.samplers = cgModel->samplers;

    for (const auto& surface : cgModel->S)
    {
        Material gltfMaterial;
        // Copying material properties
        gltfMaterial.name = surface.material.name;
        gltfMaterial.pbrMetallicRoughness.baseColorFactor = surface.material.pbrMetallicRoughness.baseColorFactor;
        gltfMaterial.pbrMetallicRoughness.metallicFactor = surface.material.pbrMetallicRoughness.metallicFactor;
        gltfMaterial.pbrMetallicRoughness.roughnessFactor = surface.material.pbrMetallicRoughness.roughnessFactor;

        if (surface.material.pbrMetallicRoughness.baseColorTexture.has_value())
        {
            gltfMaterial.pbrMetallicRoughness.baseColorTexture = surface.material.pbrMetallicRoughness.baseColorTexture.value();
        }

        if (surface.material.pbrMetallicRoughness.metallicRoughnessTexture.has_value())
        {
            gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture = surface.material.pbrMetallicRoughness.metallicRoughnessTexture.value();
        }

        // Add the completed material to GLTFData
        gltfData.materials.push_back (gltfMaterial);
    }
}
