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

    const auto& scenes = jsonData["scenes"];
    for (const auto& scene : scenes)
    {
        // Parse scene properties
        if (scene.contains ("name"))
        {
            LOG (INFO) << "Scene name: " << scene["name"];
        }
        if (scene.contains ("nodes"))
        {
            LOG (INFO) << "Scene nodes:";
            const auto& nodes = scene["nodes"];
            for (const auto& node : nodes)
            {
                LOG (INFO) << " Node index: " << node;
            }
        }
    }
}

void GLTFParser::parseSamplers()
{
    if (!jsonData.contains ("samplers"))
    {
        LOG (INFO) << "No samplers found in the GLTF file.";
        return;
    }

    const auto& samplers = jsonData["samplers"];
    for (const auto& sampler : samplers)
    {
        // Parse sampler properties
        if (sampler.contains ("magFilter"))
        {
            LOG (INFO) << "Sampler magFilter: " << sampler["magFilter"];
        }
        if (sampler.contains ("minFilter"))
        {
            LOG (INFO) << "Sampler minFilter: " << sampler["minFilter"];
        }
        if (sampler.contains ("wrapS"))
        {
            LOG (INFO) << "Sampler wrapS: " << sampler["wrapS"];
        }
        if (sampler.contains ("wrapT"))
        {
            LOG (INFO) << "Sampler wrapT: " << sampler["wrapT"];
        }
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

void GLTFParser::parseTextures()
{
    if (!jsonData.contains ("textures"))
    {
        LOG (INFO) << "No textures found in the GLTF file.";
        return;
    }

    const auto& textures = jsonData["textures"];
    for (const auto& texture : textures)
    {
        // Parse texture properties
        if (texture.contains ("sampler"))
        {
            LOG (INFO) << "Texture uses sampler: " << texture["sampler"];
        }
        if (texture.contains ("source"))
        {
            LOG (INFO) << "Texture image source: " << texture["source"];
        }
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

    const auto& nodes = jsonData["nodes"];
    for (const auto& node : nodes)
    {
        // Example of parsing a node's name, if it exists
        if (node.contains ("name"))
        {
            std::string nodeName = node["name"];
            LOG (INFO) << "Node name: " << nodeName;
        }

        // Parse transformation data (position, rotation, scale)
        if (node.contains ("translation"))
        {
            auto translation = node["translation"];
            LOG (INFO) << "Translation: X=" << translation[0] << ", Y=" << translation[1] << ", Z=" << translation[2];
        }

        if (node.contains ("rotation"))
        {
            auto rotation = node["rotation"];
            LOG (INFO) << "Rotation: X=" << rotation[0] << ", Y=" << rotation[1] << ", Z=" << rotation[2] << ", W=" << rotation[3];
        }

        if (node.contains ("scale"))
        {
            auto scale = node["scale"];
            LOG (INFO) << "Scale: X=" << scale[0] << ", Y=" << scale[1] << ", Z=" << scale[2];
        }

        // Parse matrix (if it exists)
        if (node.contains ("matrix"))
        {
            LOG (INFO) << "Node matrix:";
            const auto& matrix = node["matrix"];
            for (int i = 0; i < matrix.size(); ++i)
            {
                LOG (INFO) << " " << matrix[i] << ((i % 4 == 3) ? "\n" : "");
            }
        }

        // Parse children (if they exist)
        if (node.contains ("children"))
        {
            LOG (INFO) << "Node has children:";
            const auto& children = node["children"];
            for (const auto& child : children)
            {
                LOG (INFO) << " Child node index: " << child;
            }
        }

        // Parse references to other objects like meshes
        if (node.contains ("mesh"))
        {
            int meshIndex = node["mesh"];
            LOG (INFO) << "Node references mesh index: " << meshIndex;
        }
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

    const auto& materials = jsonData["materials"];
    for (const auto& material : materials)
    {
        // Parse material name, if it exists
        if (material.contains ("name"))
        {
            std::string materialName = material["name"];
            LOG (INFO) << "Material name: " << materialName;
        }

        // Parse the PBR (Physically-Based Rendering) metallic-roughness properties
        if (material.contains ("pbrMetallicRoughness"))
        {
            const auto& pbr = material["pbrMetallicRoughness"];

            // Base color
            if (pbr.contains ("baseColorFactor"))
            {
                const auto& color = pbr["baseColorFactor"];
                LOG (INFO) << "Base color factor: R=" << color[0] << ", G=" << color[1] << ", B=" << color[2] << ", A=" << color[3];
            }

            // Metallic and roughness factors
            if (pbr.contains ("metallicFactor"))
            {
                LOG (INFO) << "Metallic factor: " << pbr["metallicFactor"];
            }
            if (pbr.contains ("roughnessFactor"))
            {
                LOG (INFO) << "Roughness factor: " << pbr["roughnessFactor"];
            }
        }
    }
}
