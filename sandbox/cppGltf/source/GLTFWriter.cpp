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

    file << "{\n";

    writeAsset (file, data.asset);
    writeAccessors (file, data.accessors);
    writeBufferViews (file, data.bufferViews);

    /*  if (data.buffers.size())
      {
          data.buffers[0].data = data.binaryData;
      }*/

    writeBuffers (file, data.buffers);
    writeMeshes (file, data.meshes);

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
    #if 0
    if (buffer.uri.empty() || buffer.data.empty())
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
        // Assume uri is a file path and save the data to this file
        std::ofstream dataFile (buffer.uri, std::ios::binary);
        if (!dataFile.is_open())
        {
            throw std::runtime_error ("Failed to open data file: " + buffer.uri);
        }
        dataFile.write (buffer.data.data(), buffer.data.size());
        dataFile.close();
    }

    #endif
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

    file << "\"asset\": " << assetJson.dump (4) << ","
         << "\n";
}
