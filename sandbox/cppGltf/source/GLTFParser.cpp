#include "GLTFParser.h"

GLTFParser::GLTFParser (const std::string& filepath) :
    filepath_ (filepath)
{
}

bool GLTFParser::parse()
{
    std::ifstream file_stream (filepath_);
    if (!file_stream.is_open())
    {
        LOG (CRITICAL) << "Failed to open file: " << filepath_;
        return false;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    jsonData = json::parse (buffer);

    parseBuffers(); // must be first
    if (!hasBinaryData)
    {
        LOG (CRITICAL) << "Failed to load any binary data: " << filepath_;
        return false;
    }

    parseAsset();
    parseNodes();
    parseMeshes();
    parseMaterials();
    parseAccessors();
    parseBufferViews();
    parseTextures();
    parseImages();
    parseSamplers();
    parseScenes();

    return true;
}

bool GLTFParser::loadBinaryFile (const std::string& filename, Buffer& buffer)
{
    fs::path path (filename);
    if (!std::filesystem::exists (path))
    {
        LOG (CRITICAL) << "Invalid path to gltf binary: " << filename;
        return false;
    }

    std::ifstream file (filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        LOG (CRITICAL) << "Unable to open file: " << filename;
        return false;
    }

    size_t fileSize = file.tellg();
    buffer.binaryData.resize (fileSize);
    file.seekg (0, std::ios::beg);
    file.read (buffer.binaryData.data(), fileSize);
    file.close();

    hasBinaryData = true;
    return true;
}

void GLTFParser::parseAsset()
{
    if (!jsonData.contains ("asset"))
    {
        // LOG (INFO) << "No accessors found in the GLTF file.";
        return;
    }

    auto& jsonAsset = jsonData["asset"];

    if (!jsonAsset.contains ("version"))
    {
        throw std::runtime_error ("GLTF Asset must contain a version string.");
    }

    data.asset.version = jsonAsset["version"].get<std::string>();

    if (jsonAsset.contains ("generator") && jsonAsset["generator"].is_string())
    {
        data.asset.generator = jsonAsset["generator"].get<std::string>();
    }

    if (jsonAsset.contains ("minVersion") && jsonAsset["minVersion"].is_string())
    {
        data.asset.minVersion = jsonAsset["minVersion"].get<std::string>();
    }
}

void GLTFParser::parseScenes()
{
    if (!jsonData.contains ("scenes"))
    {
        // LOG (INFO) << "No scenes found in the GLTF file.";
        return;
    }

    const auto& scenesJson = jsonData["scenes"];
    for (const auto& sceneJson : scenesJson)
    {
        Scene scene; // Simplified and clear variable name
        if (sceneJson.contains ("name"))
        {
            scene.name = sceneJson["name"].get<std::string>();
        }

        if (sceneJson.contains ("nodes"))
        {
            scene.nodeIndices = sceneJson["nodes"].get<std::vector<int>>();
        }

        data.scenes.push_back (scene);
    }
}

void GLTFParser::parseSamplers()
{
    if (!jsonData.contains ("samplers"))
    {
        // LOG (INFO) << "No samplers found in the GLTF file.";
        return;
    }

    const auto& samplersJson = jsonData["samplers"];
    for (const auto& samplerJson : samplersJson)
    {
        Sampler sampler; // Create a Sampler instance

        // Parse sampler properties
        if (samplerJson.contains ("magFilter"))
        {
            sampler.magFilter = samplerJson["magFilter"].get<int>();
        }
        if (samplerJson.contains ("minFilter"))
        {
            sampler.minFilter = samplerJson["minFilter"].get<int>();
        }
        if (samplerJson.contains ("wrapS"))
        {
            sampler.wrapS = samplerJson["wrapS"].get<int>();
        }
        if (samplerJson.contains ("wrapT"))
        {
            sampler.wrapT = samplerJson["wrapT"].get<int>();
        }

        data.samplers.push_back (sampler); // Add the Sampler to the GLTFData's vector of Samplers
    }
}

void GLTFParser::parseImages()
{
    if (!jsonData.contains ("images"))
    {
        // LOG (INFO) << "No images found in the GLTF file.";
        return;
    }

    const auto& imagesJson = jsonData["images"];
    for (const auto& imageJson : imagesJson)
    {
        Image image; // Create an Image instance

        if (imageJson.contains ("uri"))
        {
            image.uri = imageJson["uri"].get<std::string>();
        }
        if (imageJson.contains ("bufferView"))
        {
            image.bufferViewIndex = imageJson["bufferView"].get<int>();
        }
        if (imageJson.contains ("mimeType"))
        {
            image.mimeType = imageJson["mimeType"].get<std::string>();
        }

        data.images.push_back (image); // Add the Image to the GLTFData's vector of Images
    }
}

void GLTFParser::parseBuffers()
{
    if (!jsonData.contains ("buffers"))
    {
        LOG (INFO) << "No buffers found in the GLTF file.";
        return;
    }

    const auto& jsonBuffers = jsonData["buffers"];
    for (const auto& jsonBuffer : jsonBuffers)
    {
        Buffer buffer;

        bool successfulLoad = false;
        if (jsonBuffer.contains ("uri"))
        {
            buffer.uri = jsonBuffer["uri"];
            std::string binaryPath = getFullPathToBinary (filepath_, buffer.uri);
            fs::path path (binaryPath);
            if (std::filesystem::exists (path))
                successfulLoad = loadBinaryFile (binaryPath, buffer);
        }
        if (jsonBuffer.contains ("byteLength"))
        {
            buffer.byteLength = jsonBuffer["byteLength"];
        }

        if (successfulLoad)
            data.buffers.push_back (buffer);
    }
}

void GLTFParser::parseAccessors()
{
    if (!jsonData.contains ("accessors"))
    {
        LOG (INFO) << "No accessors found in the GLTF file.";
        return;
    }

    const auto& jsonAccessors = jsonData["accessors"];
    for (const auto& jsonAccessor : jsonAccessors)
    {
        Accessor accessor;
        if (jsonAccessor.contains ("bufferView"))
        {
            accessor.bufferViewIndex = jsonAccessor["bufferView"];
        }
        if (jsonAccessor.contains ("byteOffset"))
        {
            accessor.byteOffset = jsonAccessor["byteOffset"];
        }
        if (jsonAccessor.contains ("componentType"))
        {
            accessor.componentType = static_cast<GLTFComponentType> (jsonAccessor["componentType"]);
        }
        if (jsonAccessor.contains ("count"))
        {
            accessor.count = jsonAccessor["count"];
        }
        if (jsonAccessor.contains ("type"))
        {
            // Assuming you have a stringToAccessorType function
            accessor.type = stringToAccessorType (jsonAccessor["type"].get<std::string>());
        }
        if (jsonAccessor.contains ("normalized"))
        {
            accessor.normalized = jsonAccessor["normalized"];
        }
        if (jsonAccessor.contains ("min"))
        {
            const auto& minArray = jsonAccessor["min"];
            for (const auto& val : minArray)
            {
                accessor.minValues.push_back (val.get<float>());
            }
        }

        if (jsonAccessor.contains ("max"))
        {
            const auto& maxArray = jsonAccessor["max"];
            for (const auto& val : maxArray)
            {
                accessor.maxValues.push_back (val.get<float>());
            }
        }

        data.accessors.push_back (accessor);
    }
}

void GLTFParser::parseNodes()
{
    if (!jsonData.contains ("nodes"))
    {
        // LOG (INFO) << "No nodes found in the GLTF file.";
        return;
    }

    const auto& nodesJson = jsonData["nodes"];
    for (const auto& nodeJson : nodesJson)
    {
        Node node; // Create a Node instance

        // Parse and set node name
        if (nodeJson.contains ("name"))
        {
            node.name = nodeJson["name"].get<std::string>();
        }

        // Directly parse matrix if it exists
        if (nodeJson.contains ("matrix"))
        {
            std::vector<float> matrixValues = nodeJson["matrix"].get<std::vector<float>>();
            Eigen::Matrix4f matrix;
            std::memcpy (matrix.data(), matrixValues.data(), 16 * sizeof (float));
         
            node.transform = Eigen::Affine3f (matrix);

            node.isMatrixMode = true;
        }
        else
        {
            if (nodeJson.contains ("translation"))
            {
                node.isMatrixMode = false;
                std::vector<float> translation = nodeJson["translation"].get<std::vector<float>>();
                node.translation = Eigen::Vector3f (translation[0], translation[1], translation[2]);
            }

            if (nodeJson.contains ("rotation"))
            {
                std::vector<float> rotation = nodeJson["rotation"].get<std::vector<float>>();
                node.rotation = Eigen::Quaternionf (rotation[3], rotation[0], rotation[1], rotation[2]);
            }
            if (nodeJson.contains ("scale"))
            {
                std::vector<float> scale = nodeJson["scale"].get<std::vector<float>>();
                node.scale = Eigen::Vector3f (scale[0], scale[1], scale[2]);
            }

            node.isMatrixMode = false;
            node.updateTransform();
        }

        // Parse and set node mesh
        if (nodeJson.contains ("mesh"))
        {
            node.mesh = nodeJson["mesh"].get<int>();
        }

        // Parse and set child nodes
        if (nodeJson.contains ("children"))
        {
            node.children = nodeJson["children"].get<std::vector<int>>();
        }

        data.nodes.push_back (node); // Add the Node to the GLTFData's vector of Nodes
    }
}

void GLTFParser::parseBufferViews()
{
    if (!jsonData.contains ("bufferViews"))
    {
        LOG (INFO) << "No bufferViews found in the GLTF file.";
        return;
    }

    const auto& jsonBufferViews = jsonData["bufferViews"];
    for (const auto& jsonBufferView : jsonBufferViews)
    {
        BufferView bufferView;
        if (jsonBufferView.contains ("buffer"))
        {
            bufferView.bufferIndex = jsonBufferView["buffer"];
        }
        if (jsonBufferView.contains ("byteOffset"))
        {
            bufferView.byteOffset = jsonBufferView["byteOffset"];
        }
        if (jsonBufferView.contains ("byteLength"))
        {
            bufferView.byteLength = jsonBufferView["byteLength"];
        }
        if (jsonBufferView.contains ("byteStride"))
        {
            bufferView.byteStride = jsonBufferView["byteStride"];
        }
        if (jsonBufferView.contains ("target"))
        {
            bufferView.target = jsonBufferView["target"];
        }

        data.bufferViews.push_back (bufferView);
    }
}

void GLTFParser::parseMeshes()
{
    if (!jsonData.contains ("meshes"))
    {
        // LOG (INFO) << "No meshes found in the GLTF file.";
        return;
    }

    const auto& jsonMeshes = jsonData["meshes"];
    for (const auto& jsonMesh : jsonMeshes)
    {
        Mesh mesh;
        if (jsonMesh.contains ("name"))
        {
            mesh.name = jsonMesh["name"];
        }

        if (jsonMesh.contains ("primitives"))
        {
            for (const auto& jsonPrimitive : jsonMesh["primitives"])
            {
                MeshPrimitive primitive;

                // Parse indices and material
                if (jsonPrimitive.contains ("indices"))
                {
                    primitive.indices = jsonPrimitive["indices"];
                }
                if (jsonPrimitive.contains ("material"))
                {
                    primitive.material = jsonPrimitive["material"];
                }

                // Parse vertex attributes
                if (jsonPrimitive.contains ("attributes"))
                {
                    const auto& attributes = jsonPrimitive["attributes"];
                    if (attributes.contains ("POSITION"))
                    {
                        primitive.attributes["POSITION"] = attributes["POSITION"];
                    }
                    if (attributes.contains ("NORMAL"))
                    {
                        primitive.attributes["NORMAL"] = attributes["NORMAL"];
                    }
                    if (attributes.contains ("TEXCOORD_0"))
                    {
                        primitive.attributes["TEXCOORD_0"] = attributes["TEXCOORD_0"];
                    }
                    if (attributes.contains ("TEXCOORD_1"))
                    {
                        primitive.attributes["TEXCOORD_1"] = attributes["TEXCOORD_1"];
                    }
                }

                mesh.primitives.push_back (primitive);
            }
        }

        data.meshes.push_back (mesh);
    }
}

void GLTFParser::parseMaterials()
{
    if (!jsonData.contains ("materials"))
    {
        // LOG (INFO) << "No materials found in the GLTF file.";
        return;
    }

    const auto& materialsJson = jsonData["materials"];
    for (const auto& materialJson : materialsJson)
    {
        Material material; // Create a Material instance

        // Parse and set material name
        if (materialJson.contains ("name"))
        {
            material.name = materialJson["name"].get<std::string>();
        }

        // Parse PBRMetallicRoughness properties
        if (materialJson.contains ("pbrMetallicRoughness"))
        {
            auto& pbrJson = materialJson["pbrMetallicRoughness"];

            if (pbrJson.contains ("baseColorTexture"))
            {
                TextureInfo baseColorTexture;

                auto& baseColorTextureJson = pbrJson["baseColorTexture"];
                if (baseColorTextureJson.contains ("index"))
                {
                    baseColorTexture.textureIndex = baseColorTextureJson["index"].get<int>();
                }

                if (baseColorTextureJson.contains ("texCoord"))
                {
                    baseColorTexture.texCoord = baseColorTextureJson["texCoord"].get<int>();
                }

                material.pbrMetallicRoughness.baseColorTexture = baseColorTexture;
            }

            // Parse base color factor
            if (pbrJson.contains ("baseColorFactor"))
            {
                std::vector<float> factor = pbrJson["baseColorFactor"].get<std::vector<float>>();
                std::copy (factor.begin(), factor.end(), material.pbrMetallicRoughness.baseColorFactor.begin());
            }

            // Parse metallic-roughness texture
            if (pbrJson.contains ("metallicRoughnessTexture"))
            {
                TextureInfo metallicRoughnessTexture;
                auto& metallicRoughnessTextureJson = pbrJson["metallicRoughnessTexture"];
                if (metallicRoughnessTextureJson.contains ("index"))
                {
                    metallicRoughnessTexture.textureIndex = metallicRoughnessTextureJson["index"].get<int>();
                }

                if (metallicRoughnessTextureJson.contains ("texCoord"))
                {
                    metallicRoughnessTexture.texCoord = metallicRoughnessTextureJson["texCoord"].get<int>();
                }

                material.pbrMetallicRoughness.metallicRoughnessTexture = metallicRoughnessTexture;
            }

            // Parse metallic factor
            if (pbrJson.contains ("metallicFactor"))
            {
                material.pbrMetallicRoughness.metallicFactor = pbrJson["metallicFactor"].get<float>();
            }

            // Parse roughness factor
            if (pbrJson.contains ("roughnessFactor"))
            {
                material.pbrMetallicRoughness.roughnessFactor = pbrJson["roughnessFactor"].get<float>();
            }
        }

        data.materials.push_back (material); // Add the Material to the GLTFData's vector of Materials
    }
}

void GLTFParser::parseTextures()
{
    if (!jsonData.contains ("textures"))
    {
        // LOG (INFO) << "No textures found in the GLTF file.";
        return;
    }

    const auto& texturesJson = jsonData["textures"];
    for (const auto& textureJson : texturesJson)
    {
        Texture texture; // Create a Texture instance

        // Parse texture properties
        if (textureJson.contains ("source"))
        {
            texture.source = textureJson["source"].get<int>();
        }

        if (textureJson.contains ("sampler"))
        {
            texture.sampler = textureJson["sampler"].get<int>();
        }

        data.textures.push_back (texture); // Add the Texture to the GLTFData's vector of Textures
    }
}

void GLTFParser::gltfStatistics()
{
    LOG (INFO) << "GLTF Data Statistics:";

    // Print asset information
    LOG (INFO) << "Asset:";
    LOG (INFO) << "  Version: " << data.asset.version;
    LOG (INFO) << "  Generator: " << data.asset.generator;

    // Print scene information
    LOG (INFO) << "Total scenes: " << data.scenes.size();
    for (const auto& scene : data.scenes)
    {
        LOG (INFO) << "  Scene Name: " << scene.name;
        // ... other scene details ...
    }

    // Print node information
    LOG (INFO) << "Total nodes: " << data.nodes.size();
    for (const auto& node : data.nodes)
    {
        LOG (INFO) << "  Node Name: " << node.name;
        // ... other node details ...
    }

    // Print material information
    LOG (INFO) << "Total materials: " << data.materials.size();
    for (const auto& material : data.materials)
    {
        LOG (INFO) << "  Material Name: " << material.name;
        // ... other material details ...
    }

    // Print texture information
    LOG (INFO) << "Total textures: " << data.textures.size();
    // ... similarly for textures ...

    // Print sampler information
    LOG (INFO) << "Total samplers: " << data.samplers.size();
    // ... similarly for samplers ...

    // Print image information
    LOG (INFO) << "Total images: " << data.images.size();
    for (const auto& image : data.images)
    {
        LOG (INFO) << "  Image URI: " << image.uri;
        // ... other image details ...
    }

    // Print buffer information
    LOG (INFO) << "Total buffers: " << data.buffers.size();
    for (const auto& buffer : data.buffers)
    {
        LOG (INFO) << "  Buffer Size: " << buffer.byteLength;
        // ... other buffer details ...
    }

    // Print bufferView information
    LOG (INFO) << "Total bufferViews: " << data.bufferViews.size();
    for (const auto& bufferView : data.bufferViews)
    {
        LOG (INFO) << "  BufferView - Buffer: " << bufferView.bufferIndex;
        LOG (INFO) << "  Byte Length: " << bufferView.byteLength;
        LOG (INFO) << "  Byte Offset: " << bufferView.byteOffset;
        // ... other bufferView details ...
    }

    LOG (INFO) << "Total accessors: " << data.accessors.size();
    for (const auto& accessor : data.accessors)
    {
        LOG (INFO) << "  Accessor Component Type: " << componentTypeToString (accessor.componentType);
        // ... other accessor details ...
    }
}
