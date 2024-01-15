#include "ModelBuilder.h"
#include "cgmodel/cgModel.h"

using Eigen::Vector3f;

constexpr int TRI_INDICES = 3;

cgModelPtr ModelBuilder::createCgModel()
{
    return buildModelList();
}

cgModelPtr ModelBuilder::buildModelList()
{
    ModelList cgModels;

    for (const auto& mesh : data.meshes)
    {
        
        for (const auto& primitive : mesh.primitives)
        {
            // one cgModel for each primitive
            // each model can have only 1 surface
            cgModelPtr model = cgModel::create();

            // get the surface indices
            cgModelSurface surface;
            const Accessor& indexAccessor = data.accessors[primitive.indices];
            getTriangleIndices (surface.F, indexAccessor);

            // get vertex attributes
            if (primitive.attributes.find ("POSITION") != primitive.attributes.end())
            {
                int accessorIndex = primitive.attributes.at ("POSITION");
                const Accessor& accessor = data.accessors[accessorIndex];

                getVertexFloatAttribute (model->V, accessor);
            }
            if (primitive.attributes.find ("NORMAL") != primitive.attributes.end())
            {
                int accessorIndex = primitive.attributes.at ("NORMAL");
                const Accessor& accessor = data.accessors[accessorIndex];

                getVertexFloatAttribute (model->N, accessor);
            }
            if (primitive.attributes.find ("TEXCOORD_0") != primitive.attributes.end())
            {
                int accessorIndex = primitive.attributes.at ("TEXCOORD_0");
                const Accessor& accessor = data.accessors[accessorIndex];

                getVertexFloatAttribute (model->UV, accessor);
            }
            if (primitive.attributes.find ("TEXCOORD_1") != primitive.attributes.end())
            {
                int accessorIndex = primitive.attributes.at ("TEXCOORD_1");
                const Accessor& accessor = data.accessors[accessorIndex];

                getVertexFloatAttribute (model->UV, accessor);
            }

            model->S.push_back (surface);

            cgModels.push_back (model);
        }
    }

    return cgModels.empty() ? nullptr : (cgModels.size() > 1 ? forgeIntoOne (cgModels) : cgModels[0]);
}

void ModelBuilder::getTriangleIndices (MatrixXu& matrix, const Accessor& accessor)
{
    size_t numTriangles = accessor.count / 3;
    matrix.resize (3, numTriangles);

    // Retrieve the buffer view for the accessor
    const BufferView& bufferView = data.bufferViews[accessor.bufferViewIndex];

    // Calculate the starting position of the index data in the binary buffer
    size_t dataStart = bufferView.byteOffset + accessor.byteOffset;

    LOG (INFO) << "Triangle count: " << numTriangles;

    GLTFComponentType componentType = accessor.componentType;
    size_t stride = getComponentSize (componentType);

    for (size_t i = 0; i < numTriangles; ++i)
    {
        switch (componentType)
        {
            case GLTFComponentType::UNSIGNED_BYTE:
            {
                const uint8_t* indicesStart = reinterpret_cast<const uint8_t*> (&data.binaryData[dataStart + i * TRI_INDICES * stride]);
                for (size_t j = 0; j < TRI_INDICES; ++j)
                {
                    matrix (j, i) = static_cast<unsigned int> (indicesStart[j]);
                }
                break;
            }
            case GLTFComponentType::UNSIGNED_SHORT:
            {
                const uint16_t* indicesStart = reinterpret_cast<const uint16_t*> (&data.binaryData[dataStart + i * TRI_INDICES * stride]);
                for (size_t j = 0; j < TRI_INDICES; ++j)
                {
                    matrix (j, i) = static_cast<unsigned int> (indicesStart[j]);
                }
                break;
            }
            case GLTFComponentType::UNSIGNED_INT:
            {
                const uint32_t* indicesStart = reinterpret_cast<const uint32_t*> (&data.binaryData[dataStart + i * TRI_INDICES * stride]);
                for (size_t j = 0; j < TRI_INDICES; ++j)
                {
                    matrix (j, i) = indicesStart[j];
                }
                break;
            }
            default:
                break;
        }
    }

#if 0
    for (int i = 0; i < matrix.cols(); ++i)
    {
        
        Vector3u tri = matrix.col (i);
        LOG (INFO) << tri.x() << ", " << tri.y() << ", " << tri.z();
    }
#endif
}

void ModelBuilder::getVertexFloatAttribute (Eigen::MatrixXf& matrix, const Accessor& accessor)
{
    const BufferView& bufferView = data.bufferViews[accessor.bufferViewIndex];

    size_t numComponents = getNumberOfComponents (accessor.type);
    uint32_t count = accessor.count;

    matrix.resize (numComponents, count);

    size_t offset = bufferView.byteOffset + accessor.byteOffset;
    std::memcpy (matrix.data(), data.binaryData.data() + offset, count * numComponents * sizeof (float));
}

cgModelPtr ModelBuilder::forgeIntoOne (const ModelList& models)
{
    // Initialize counters for total vertices and triangles.
    uint32_t totalVertices = 0;
    uint32_t totalTriangles = 0;

    // Reserve space for vertex offsets.
    std::vector<uint32_t> vertexOffsets;
    vertexOffsets.reserve (models.size());
    vertexOffsets.push_back (totalVertices);

    // Create a shared pointer for the flattened mesh.
    auto flattenedModel = std::make_shared<cgModel>();

    // Iterate over the input meshes.
    for (const auto& m : models)
    {
        // Update the total vertices and triangles counters.
        totalVertices += m->V.cols();
        vertexOffsets.push_back (totalVertices);

        // each mesh can have only 1 surface!
        assert (m->S.size() == 1);

        totalTriangles += m->S[0].triangleCount();

        // Move surfaces from input mesh to the flattened mesh.
        flattenedModel->S.emplace_back (std::move (m->S[0]));
    }

    // Resize the vertex and uv matrices of the flattened mesh.
    flattenedModel->V.resize (3, totalVertices);

    // FIXME some meshes don't have UV data (Storm Trooper for example)
    flattenedModel->UV.resize (2, totalVertices);

    // Copy vertex and uv data from input meshes to the flattened mesh.
    for (uint32_t index = 0; index < models.size(); ++index)
    {
        const auto& mesh = models[index];
        std::memcpy (flattenedModel->V.data() + vertexOffsets[index] * 3, mesh->V.data(), mesh->vertexCount() * 3 * sizeof (float));

        // some meshes don't have UV data
        if (mesh->UV.cols())
            std::memcpy (flattenedModel->UV.data() + vertexOffsets[index] * 2, mesh->UV.data(), mesh->vertexCount() * 2 * sizeof (float));
    }

    // Update triangle indices in the flattened mesh with new vertex offsets.
    for (uint32_t index = 0; index < flattenedModel->S.size(); ++index)
    {
        auto& s = flattenedModel->S[index];
        uint32_t vertexOffset = vertexOffsets[index];
        auto& tris = s.indices();

        // Update each triangle's indices with the corresponding vertex offset.
        for (int i = 0; i < s.triangleCount(); i++)
        {
            Vector3u tri = tris.col (i);
            for (int j = 0; j < 3; j++)
                tri[j] += vertexOffset;
            tris.col (i) = tri;
        }
    }

    return flattenedModel;
}
