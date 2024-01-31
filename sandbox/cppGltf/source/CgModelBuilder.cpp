#include "CgModelBuilder.h"
#include "cgmodel/CgModel.h"

using Eigen::Vector3f;

constexpr int TRI_INDICES = 3;

CgModelPtr CgModelBuilder::createCgModel()
{
    return buildModelList();
}

void CgModelBuilder::applyNodeTransforms (const Node& node, const Eigen::Affine3f& parentTransform, const std::vector<Node>& nodes, std::vector<CgModelPtr>& cgModels)
{
    Eigen::Affine3f currentTransform;

    if (node.isMatrixMode)
    {
        // Matrix mode: Use the transformation matrix directly
        currentTransform = parentTransform * node.transform;
    }
    else
    {
        // TRS mode: Construct the transformation matrix from translation, rotation, and scale
        currentTransform = parentTransform * (Eigen::Translation3f (node.translation) * node.rotation * Eigen::Scaling (node.scale));
    }

    if (node.mesh)
    {
        // Use the existing transformVertices function to apply the transformation
        CgModelPtr& model = cgModels[node.mesh.value()];
        model->transformVertices (currentTransform);
    }

    // Recursively apply transformations for each child node
    for (int childIndex : node.children)
    {
        applyNodeTransforms (nodes[childIndex], currentTransform, nodes, cgModels);
    }
}

bool CgModelBuilder::isRootNode (const Node& node, const std::vector<Node>& nodes)
{
    // Iterate through all nodes to check if our node is a child of any
    for (const auto& potentialParent : nodes)
    {
        // Check if the current node's index is in the potentialParent's children list
        if (std::find (potentialParent.children.begin(), potentialParent.children.end(), &node - &nodes[0]) != potentialParent.children.end())
        {
            return false; // Found the node in a child list, so it's not a root node
        }
    }
    return true; // The node is not a child of any node, so it's a root node
}

CgModelPtr CgModelBuilder::buildModelList()
{
    ModelList cgModels;

    for (const auto& mesh : data.meshes)
    {
        for (const auto& primitive : mesh.primitives)
        {
            // one CgModel for each primitive
            // each model can have only 1 surface
            CgModelPtr model = CgModel::create();

            // get the surface indices
            CgModelSurface surface;
            if (primitive.indices != INVALID_INDEX)
            {
                const Accessor& indexAccessor = data.accessors[primitive.indices];
                getTriangleIndices (surface.F, indexAccessor);

                // get the material settings
                if (primitive.material != INVALID_INDEX && primitive.material < data.materials.size())
                {
                    surface.material = data.materials[primitive.material];
                }
            }

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

                getVertexFloatAttribute (model->UV0, accessor);
            }
            if (primitive.attributes.find ("TEXCOORD_1") != primitive.attributes.end())
            {
                int accessorIndex = primitive.attributes.at ("TEXCOORD_1");
                const Accessor& accessor = data.accessors[accessorIndex];

                getVertexFloatAttribute (model->UV1, accessor);
            }

            model->S.push_back (surface);

            model->images = data.images;
            model->textures = data.textures;
            model->samplers = data.samplers;

            cgModels.push_back (model);
        }
    }

    // Iterate through all nodes and apply transformations
    for (const Node& node : data.nodes)
    {
        if (isRootNode (node, data.nodes))
        {
            applyNodeTransforms (node, Eigen::Affine3f::Identity(), data.nodes, cgModels);
        }
    }
    return cgModels.empty() ? nullptr : (cgModels.size() > 1 ? forgeIntoOne (cgModels) : cgModels[0]);
}

void CgModelBuilder::getTriangleIndices (MatrixXu& matrix, const Accessor& accessor)
{
    if (accessor.count < 3)
        throw std::runtime_error ("No triangle indices");

    size_t numTriangles = accessor.count / 3;
    matrix.resize (3, numTriangles);

    // Retrieve the buffer view for the accessor
    const BufferView& bufferView = data.bufferViews[accessor.bufferViewIndex];
    const Buffer& buffer = data.buffers[bufferView.bufferIndex];

    // Calculate the starting position of the index data in the binary buffer
    size_t dataStart = bufferView.byteOffset + accessor.byteOffset;

    GLTFComponentType componentType = accessor.componentType;
    size_t stride = getComponentSize (componentType);

    for (size_t i = 0; i < numTriangles; ++i)
    {
        switch (componentType)
        {
            case GLTFComponentType::UNSIGNED_BYTE:
            {
                const uint8_t* indicesStart = reinterpret_cast<const uint8_t*> (&buffer.binaryData[dataStart + i * TRI_INDICES * stride]);
                for (size_t j = 0; j < TRI_INDICES; ++j)
                {
                    matrix (j, i) = static_cast<unsigned int> (indicesStart[j]);
                }
                break;
            }
            case GLTFComponentType::UNSIGNED_SHORT:
            {
                const uint16_t* indicesStart = reinterpret_cast<const uint16_t*> (&buffer.binaryData[dataStart + i * TRI_INDICES * stride]);
                for (size_t j = 0; j < TRI_INDICES; ++j)
                {
                    matrix (j, i) = static_cast<unsigned int> (indicesStart[j]);
                }
                break;
            }
            case GLTFComponentType::UNSIGNED_INT:
            {
                const uint32_t* indicesStart = reinterpret_cast<const uint32_t*> (&buffer.binaryData[dataStart + i * TRI_INDICES * stride]);
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

void CgModelBuilder::getVertexFloatAttribute (MatrixXf& matrix, const Accessor& accessor)
{
    const BufferView& bufferView = data.bufferViews[accessor.bufferViewIndex];
    const Buffer& buffer = data.buffers[bufferView.bufferIndex];

    size_t numComponents = getNumberOfComponents (accessor.type);
    uint32_t count = accessor.count;

    matrix.resize (numComponents, count);

    size_t offset = bufferView.byteOffset + accessor.byteOffset;
    std::memcpy (matrix.data(), buffer.binaryData.data() + offset, count * numComponents * sizeof (float));
}

CgModelPtr CgModelBuilder::forgeIntoOne (const ModelList& models)
{
    // Initialize counters for total vertices and triangles.
    uint32_t totalVertices = 0;
    uint32_t totalTriangles = 0;

    // Reserve space for vertex offsets.
    std::vector<uint32_t> vertexOffsets;
    vertexOffsets.reserve (models.size());
    vertexOffsets.push_back (totalVertices);

    // Create a shared pointer for the flattened mesh.
    auto flattenedModel = std::make_shared<CgModel>();

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
    // and what about UV1?
    flattenedModel->UV0.resize (2, totalVertices);

    // Copy vertex and uv data from input meshes to the flattened mesh.
    for (uint32_t index = 0; index < models.size(); ++index)
    {
        const auto& mesh = models[index];
        std::memcpy (flattenedModel->V.data() + vertexOffsets[index] * 3, mesh->V.data(), mesh->vertexCount() * 3 * sizeof (float));

        // some meshes don't have UV data
        if (mesh->UV0.cols())
            std::memcpy (flattenedModel->UV0.data() + vertexOffsets[index] * 2, mesh->UV0.data(), mesh->vertexCount() * 2 * sizeof (float));

        // FIXME what about UV1?
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

    flattenedModel->images = data.images;
    flattenedModel->textures = data.textures;
    flattenedModel->samplers = data.samplers;

    return flattenedModel;
}
