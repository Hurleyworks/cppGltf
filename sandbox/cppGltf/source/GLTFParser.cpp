#include "GLTFParser.h"

GLTFParser::GLTFParser (const std::string& filepath) :
    filepath_ (filepath)
{
}

void GLTFParser::parse()
{
    std::ifstream file_stream (filepath_);
    if (!file_stream.is_open())
    {
        LOG (CRITICAL) << "Failed to open file: " << filepath_;
        return;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    jsonData = json::parse (buffer);

    parseBuffers(); // must be first
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
}

void GLTFParser::loadBinaryFile (const std::string& filename)
{
    fs::path path (filename);
    if (!std::filesystem::exists (path))
        throw std::runtime_error ("Invalid path to gltf binary: " + filename);

    std::ifstream file (filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error ("Unable to open file: " + filename);
    }

    size_t fileSize = file.tellg();
    data.binaryData.resize (fileSize);
    file.seekg (0, std::ios::beg);
    file.read (data.binaryData.data(), fileSize);
    file.close();
}

void GLTFParser::parseAsset()
{
    if (!jsonData.contains ("asset"))
    {
        LOG (INFO) << "No accessors found in the GLTF file.";
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
        LOG (INFO) << "No scenes found in the GLTF file.";
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
        LOG (INFO) << "No samplers found in the GLTF file.";
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
        LOG (INFO) << "No images found in the GLTF file.";
        return;
    }

    const auto& images = jsonData["images"];
    for (const auto& image : images)
    {
        // Parse image properties
        if (image.contains ("uri"))
        {
            std::string uri = image["uri"];
            LOG (INFO) << "Image URI: " << uri;

            // Handle the image URI
            // If it's a relative path, it references an external file.
            // If it's a data URI, it's an embedded image (usually base64 encoded).
        }

        // GLTF 2.0 also allows images to reference a bufferView
        if (image.contains ("bufferView"))
        {
            int bufferViewIndex = image["bufferView"];
            LOG (INFO) << "Image uses bufferView: " << bufferViewIndex;

            // Additional processing can be done here to retrieve the data from the bufferView
        }
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

        if (jsonBuffer.contains ("uri"))
        {
            buffer.uri = jsonBuffer["uri"];
            loadBinaryFile (getFullPathToBinary (filepath_, buffer.uri));
        }
        if (jsonBuffer.contains ("byteLength"))
        {
            buffer.byteLength = jsonBuffer["byteLength"];
        }

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

        data.accessors.push_back (accessor);
    }
}

void GLTFParser::parseNodes()
{
    if (!jsonData.contains ("nodes"))
    {
        LOG (INFO) << "No nodes found in the GLTF file.";
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
            for (int i = 0; i < 16; ++i)
                matrix (i / 4, i % 4) = matrixValues[i];

            node.transform = Eigen::Affine3f (matrix);
        }
        else
        {
            // Parse and set node transform components
            if (nodeJson.contains ("translation"))
            {
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
        LOG (INFO) << "No meshes found in the GLTF file.";
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
        LOG (INFO) << "No materials found in the GLTF file.";
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
                    baseColorTexture.index = baseColorTextureJson["index"].get<int>();
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
                    metallicRoughnessTexture.index = metallicRoughnessTextureJson["index"].get<int>();
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
        LOG (INFO) << "No textures found in the GLTF file.";
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
