#include "GLTFWriter.h"

using Eigen::Vector3f;

constexpr int TRI_INDICES = 3;

void GLTFWriter::write (const std::string& filepath, const GLTFData&& data)
{
    std::ofstream file (filepath);
    if (!file.is_open())
    {
        throw std::runtime_error ("Failed to open file: " + filepath);
    }

    this->filePath = filepath;

    file << "{\n";

    if (data.accessors.size())
        writeAccessors (file, data.accessors);

    if (data.bufferViews.size())
        writeBufferViews (file, data.bufferViews);

    if (data.buffers.size())
        writeBuffers (file, data.buffers);

    if (data.materials.size())
        writeMaterials (file, data.materials);

    if (data.images.size())
        writeImages (file, data.images);

    if (data.textures.size())
        writeTextures (file, data.textures);

    if (data.samplers.size())
        writeSamplers (file, data.samplers);

    if (data.scenes.size())
        writeScenes (file, data.scenes);

    if (data.nodes.size())
        writeNodes (file, data.nodes);

    if (data.meshes.size())
        writeMeshes (file, data.meshes);

    // last has no comma
    writeAsset (file, data.asset);

    file << "\n}";
    file.close();
}

void GLTFWriter::writeAccessors (std::ofstream& file, const std::vector<Accessor>& accessors)
{
    json j;

    for (const auto& accessor : accessors)
    {
        json accessorJson;
        accessorJson["bufferView"] = accessor.bufferViewIndex;
        accessorJson["byteOffset"] = accessor.byteOffset;
        accessorJson["componentType"] = static_cast<int> (accessor.componentType);
        accessorJson["count"] = accessor.count;
        accessorJson["type"] = accessorTypeToString (accessor.type);
        accessorJson["normalized"] = accessor.normalized;

        if (!accessor.minValues.empty())
        {
            accessorJson["min"] = accessor.minValues;
        }

        if (!accessor.maxValues.empty())
        {
            accessorJson["max"] = accessor.maxValues;
        }

        j["accessors"].push_back (accessorJson);
    }

    file << "\"accessors\": " << j["accessors"].dump (4) << ","
         << "\n";
}

void GLTFWriter::writeBufferViews (std::ofstream& file, const std::vector<BufferView>& bufferViews)
{
    json j;

    for (const auto& bufferView : bufferViews)
    {
        json bufferViewJson;
        bufferViewJson["buffer"] = bufferView.bufferIndex;
        bufferViewJson["byteOffset"] = bufferView.byteOffset;
        bufferViewJson["byteLength"] = bufferView.byteLength;

        // Only add byteStride if it's not zero
        if (bufferView.byteStride > 0)
        {
            bufferViewJson["byteStride"] = bufferView.byteStride;
        }

        // Only add target if it's not zero
        if (bufferView.target > 0)
        {
            bufferViewJson["target"] = bufferView.target;
        }

        j["bufferViews"].push_back (bufferViewJson);
    }

    file << "\"bufferViews\": " << j["bufferViews"].dump (4) << ","
         << "\n";
}

void GLTFWriter::writeBuffers (std::ofstream& file, const std::vector<Buffer>& buffers)
{
    json j;
    for (const auto& buffer : buffers)
    {
        saveBufferData (buffer);

        json bufferJson;
        bufferJson["byteLength"] = buffer.byteLength;
        bufferJson["uri"] = buffer.uri; // Assuming URI is already set correctly
        j["buffers"].push_back (bufferJson);
    }

    file << "\"buffers\": " << j["buffers"].dump (4) << ","
         << "\n";
}

void GLTFWriter::saveBufferData (const Buffer& buffer)
{
    if (buffer.uri.empty() || buffer.binaryData.empty())
    {
        return; // Nothing to do if URI is empty or data is empty
    }

    if (buffer.uri.find ("data:") == 0)
    {
        // Handle data URI (base64 encoding)
        // This part is optional and depends on whether you want to support embedding data directly in the GLTF file
    }
    else
    {
        std::string fullBinPath = convertToCustomBinPath (filePath, buffer.uri);
        std::ofstream dataFile (fullBinPath, std::ios::binary);
        if (!dataFile.is_open())
        {
            throw std::runtime_error ("Failed to open data file: " + buffer.uri);
        }
        dataFile.write (buffer.binaryData.data(), buffer.binaryData.size());
        dataFile.close();
    }
}

void GLTFWriter::writeMeshes (std::ofstream& file, const std::vector<Mesh>& meshes)
{
    json j;

    for (const auto& mesh : meshes)
    {
        json meshJson;
        if (!mesh.name.empty())
        {
            meshJson["name"] = mesh.name;
        }

        for (const auto& primitive : mesh.primitives)
        {
            json primitiveJson;
            if (primitive.indices != -1)
            {
                primitiveJson["indices"] = primitive.indices;
            }
            if (primitive.material != -1)
            {
                primitiveJson["material"] = primitive.material;
            }
            primitiveJson["mode"] = static_cast<int> (primitive.mode);

            json attributesJson;
            for (const auto& attribute : primitive.attributes)
            {
                attributesJson[attribute.first] = attribute.second;
            }
            primitiveJson["attributes"] = attributesJson;

            meshJson["primitives"].push_back (primitiveJson);
        }

        j["meshes"].push_back (meshJson);
    }

    file << "\"meshes\": " << j["meshes"].dump (4) << ","
         << "\n";
}

void GLTFWriter::writeAsset (std::ofstream& file, const Asset& asset)
{
    json assetJson;
    assetJson["version"] = asset.version; // version is required

    if (!asset.generator.empty())
    {
        assetJson["generator"] = asset.generator;
    }

    if (!asset.minVersion.empty())
    {
        assetJson["minVersion"] = asset.minVersion;
    }

    file << "\"asset\": " << assetJson.dump (4)
         << "\n";
}

void GLTFWriter::writeScenes (std::ofstream& file, const std::vector<Scene>& scenes)
{
    json scenesJson = json::array();

    for (const auto& scene : scenes)
    {
        json sceneJson;
        sceneJson["name"] = scene.name; // Assuming 'name' is a property of Scene

        // Serializing nodes array - assuming 'nodes' is a vector of integers representing node indices
        if (!scene.nodeIndices.empty())
        {
            sceneJson["nodes"] = scene.nodeIndices;
        }

        scenesJson.push_back (sceneJson);
    }

    file << "\"scenes\": " << scenesJson.dump (4) << ","
         << "\n";
}

void GLTFWriter::writeNodes (std::ofstream& file, const std::vector<Node>& nodes)
{
    json nodesJson = json::array();

    for (const auto& node : nodes)
    {
        json nodeJson;

        // Name
        if (!node.name.empty())
        {
            nodeJson["name"] = node.name;
        }

        // Children
        if (!node.children.empty())
        {
            nodeJson["children"] = node.children;
        }

        // Mesh
        if (node.mesh.has_value())
        {
            nodeJson["mesh"] = node.mesh.value();
        }

        // Camera
        if (node.camera.has_value())
        {
            nodeJson["camera"] = node.camera.value();
        }

        // Transformation: Translation, Rotation, Scale
        if (!(node.translation.isApprox (Eigen::Vector3f::Zero())))
        {
            nodeJson["translation"] = {node.translation.x(), node.translation.y(), node.translation.z()};
        }
        if (!(node.rotation.isApprox (Eigen::Quaternionf::Identity())))
        {
            nodeJson["rotation"] = {node.rotation.w(), node.rotation.x(), node.rotation.y(), node.rotation.z()};
        }
        if (!(node.scale.isApprox (Eigen::Vector3f (1.0f, 1.0f, 1.0f))))
        {
            nodeJson["scale"] = {node.scale.x(), node.scale.y(), node.scale.z()};
        }

        // Transformation: Matrix
        if (!(node.transform.isApprox (Eigen::Affine3f::Identity())))
        {
            std::vector<float> matrix;
            matrix.reserve (16);
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    matrix.push_back (node.transform (i, j));
                }
            }
            nodeJson["matrix"] = matrix;
        }

        nodesJson.push_back (nodeJson);
    }

    file << "\"nodes\": " << nodesJson.dump (4) << ","
         << "\n";
}

void GLTFWriter::writeMaterials (std::ofstream& file, const std::vector<Material>& materials)
{
    json materialsJson = json::array();

    for (const auto& material : materials)
    {
        json materialJson;
        materialJson["name"] = material.name;

        // PBR Metallic-Roughness properties
        json pbrJson;
        pbrJson["baseColorFactor"] = material.pbrMetallicRoughness.baseColorFactor;
        pbrJson["metallicFactor"] = material.pbrMetallicRoughness.metallicFactor;
        pbrJson["roughnessFactor"] = material.pbrMetallicRoughness.roughnessFactor;

        // Base Color Texture
        if (material.pbrMetallicRoughness.baseColorTexture.has_value())
        {
            json textureJson;
            textureJson["index"] = material.pbrMetallicRoughness.baseColorTexture->textureIndex;
            textureJson["texCoord"] = material.pbrMetallicRoughness.baseColorTexture->texCoord;
            pbrJson["baseColorTexture"] = textureJson;
        }

        // Metallic-Roughness Texture
        if (material.pbrMetallicRoughness.metallicRoughnessTexture.has_value())
        {
            json textureJson;
            textureJson["index"] = material.pbrMetallicRoughness.metallicRoughnessTexture->textureIndex;
            textureJson["texCoord"] = material.pbrMetallicRoughness.metallicRoughnessTexture->texCoord;
            pbrJson["metallicRoughnessTexture"] = textureJson;
        }

        materialJson["pbrMetallicRoughness"] = pbrJson;

        // Normal, Occlusion, and Emissive Textures
        auto addTextureInfo = [&] (const std::string& key, const std::optional<TextureInfo>& textureInfo)
        {
            if (textureInfo.has_value())
            {
                json textureJson;
                textureJson["index"] = textureInfo->textureIndex;
                textureJson["texCoord"] = textureInfo->texCoord;
                materialJson[key] = textureJson;
            }
        };

        addTextureInfo ("normalTexture", material.normalTexture);
        addTextureInfo ("occlusionTexture", material.occlusionTexture);
        addTextureInfo ("emissiveTexture", material.emissiveTexture);

        materialsJson.push_back (materialJson);
    }

    file << "\"materials\": " << materialsJson.dump (4) << ","
         << "\n";
}

void GLTFWriter::writeTextures (std::ofstream& file, const std::vector<Texture>& textures)
{
    json texturesJson = json::array();

    for (const auto& texture : textures)
    {
        json textureJson;

        // Texture source (Image index)
        if (texture.source != INVALID_INDEX)
        {
            textureJson["source"] = texture.source;
        }

        // Texture sampler
        if (texture.sampler != INVALID_INDEX)
        {
            textureJson["sampler"] = texture.sampler;
        }

        texturesJson.push_back (textureJson);
    }

    file << "\"textures\": " << texturesJson.dump (4) << ","
         << "\n";
}

void GLTFWriter::writeSamplers (std::ofstream& file, const std::vector<Sampler>& samplers)
{
    json samplersJson = json::array();

    for (const auto& sampler : samplers)
    {
        json samplerJson;

        // Magnification filter
        if (sampler.magFilter != INVALID_INDEX)
        {
            samplerJson["magFilter"] = sampler.magFilter;
        }

        // Minification filter
        if (sampler.minFilter != INVALID_INDEX)
        {
            samplerJson["minFilter"] = sampler.minFilter;
        }

        // Wrapping mode for s-coordinate
        samplerJson["wrapS"] = sampler.wrapS;

        // Wrapping mode for t-coordinate
        samplerJson["wrapT"] = sampler.wrapT;

        samplersJson.push_back (samplerJson);
    }

    file << "\"samplers\": " << samplersJson.dump (4) << ","
         << "\n";
}

void GLTFWriter::writeImages (std::ofstream& file, const std::vector<Image>& images)
{
    json imagesJson = json::array();

    for (const auto& image : images)
    {
        json imageJson;

        // Image URI
        if (!image.uri.empty())
        {
            imageJson["uri"] = image.uri;
        }

        // BufferView index
        if (image.bufferView != INVALID_INDEX)
        {
            imageJson["bufferView"] = image.bufferView;
        }

        // MIME type
        if (!image.mimeType.empty())
        {
            imageJson["mimeType"] = image.mimeType;
        }

        imagesJson.push_back (imageJson);
    }

    file << "\"images\": " << imagesJson.dump (4) << ","
         << "\n";
}
