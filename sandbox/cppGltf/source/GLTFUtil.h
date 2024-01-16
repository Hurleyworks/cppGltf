#pragma once

#include <mace_core/mace_core.h>

namespace
{
    enum class GLTFComponentType
    {
        BYTE = 5120,
        UNSIGNED_BYTE = 5121,
        SHORT = 5122,
        UNSIGNED_SHORT = 5123,
        UNSIGNED_INT = 5125,
        FLOAT = 5126
    };

    size_t getComponentSize (GLTFComponentType type)
    {
        switch (type)
        {
            case GLTFComponentType::BYTE:
            case GLTFComponentType::UNSIGNED_BYTE:
                return 1;
            case GLTFComponentType::SHORT:
            case GLTFComponentType::UNSIGNED_SHORT:
                return 2;
            case GLTFComponentType::UNSIGNED_INT:
                return 4;
            case GLTFComponentType::FLOAT:
                return 4;
            default:
                throw std::runtime_error ("Unsupported component type");
        }
    }

    // Optional: Function to convert the enum to a human-readable string
    std::string componentTypeToString (GLTFComponentType type)
    {
        switch (type)
        {
            case GLTFComponentType::BYTE:
                return "BYTE";
            case GLTFComponentType::UNSIGNED_BYTE:
                return "UNSIGNED_BYTE";
            case GLTFComponentType::SHORT:
                return "SHORT";
            case GLTFComponentType::UNSIGNED_SHORT:
                return "UNSIGNED_SHORT";
            case GLTFComponentType::UNSIGNED_INT:
                return "UNSIGNED_INT";
            case GLTFComponentType::FLOAT:
                return "FLOAT";
            default:
                return "UNKNOWN";
        }
    }

    enum class GLTFAccessorType
    {
        SCALAR,
        VEC2,
        VEC3,
        VEC4,
        MAT2,
        MAT3,
        MAT4
    };

    // Optional: Function to convert the enum to a human-readable string
    std::string accessorTypeToString (GLTFAccessorType type)
    {
        switch (type)
        {
            case GLTFAccessorType::SCALAR:
                return "SCALAR";
            case GLTFAccessorType::VEC2:
                return "VEC2";
            case GLTFAccessorType::VEC3:
                return "VEC3";
            case GLTFAccessorType::VEC4:
                return "VEC4";
            case GLTFAccessorType::MAT2:
                return "MAT2";
            case GLTFAccessorType::MAT3:
                return "MAT3";
            case GLTFAccessorType::MAT4:
                return "MAT4";
            default:
                return "UNKNOWN";
        }
    }

    GLTFAccessorType stringToAccessorType (const std::string& typeString)
    {
        if (typeString == "SCALAR")
        {
            return GLTFAccessorType::SCALAR;
        }
        else if (typeString == "VEC2")
        {
            return GLTFAccessorType::VEC2;
        }
        else if (typeString == "VEC3")
        {
            return GLTFAccessorType::VEC3;
        }
        else if (typeString == "VEC4")
        {
            return GLTFAccessorType::VEC4;
        }
        else if (typeString == "MAT2")
        {
            return GLTFAccessorType::MAT2;
        }
        else if (typeString == "MAT3")
        {
            return GLTFAccessorType::MAT3;
        }
        else if (typeString == "MAT4")
        {
            return GLTFAccessorType::MAT4;
        }
        else
        {
            // Handle unknown types, maybe log an error or throw an exception
            LOG (CRITICAL) << "Unknown accessor type: " << typeString;
            return GLTFAccessorType::SCALAR; // Default or error value
        }
    }

    size_t getNumberOfComponents (GLTFAccessorType type)
    {
        switch (type)
        {
            case GLTFAccessorType::SCALAR:
                return 1;
            case GLTFAccessorType::VEC2:
                return 2;
            case GLTFAccessorType::VEC3:
                return 3;
            case GLTFAccessorType::VEC4:
                return 4;
            case GLTFAccessorType::MAT2:
                return 4;
            case GLTFAccessorType::MAT3:
                return 9;
            case GLTFAccessorType::MAT4:
                return 16;
            default:
                throw std::runtime_error ("Unsupported accessor type");
        }
    }

    enum class GLTFMeshMode
    {
        POINTS = 0,
        LINES = 1,
        LINE_LOOP = 2,
        LINE_STRIP = 3,
        TRIANGLES = 4,
        TRIANGLE_STRIP = 5,
        TRIANGLE_FAN = 6
    };

    // Optional: Function to convert the enum to a human-readable string
    std::string meshModeToString (GLTFMeshMode mode)
    {
        switch (mode)
        {
            case GLTFMeshMode::POINTS:
                return "POINTS";
            case GLTFMeshMode::LINES:
                return "LINES";
            case GLTFMeshMode::LINE_LOOP:
                return "LINE_LOOP";
            case GLTFMeshMode::LINE_STRIP:
                return "LINE_STRIP";
            case GLTFMeshMode::TRIANGLES:
                return "TRIANGLES";
            case GLTFMeshMode::TRIANGLE_STRIP:
                return "TRIANGLE_STRIP";
            case GLTFMeshMode::TRIANGLE_FAN:
                return "TRIANGLE_FAN";
            default:
                return "UNKNOWN";
        }
    }

    struct BufferView
    {
        int bufferIndex = -1;  // Index of the buffer
        size_t byteOffset = 0; // Offset into the buffer in bytes
        size_t byteLength = 0; // Total length of the bufferView in bytes
        size_t byteStride = 0; // The stride, in bytes, between consecutive elements
        int target = 0;        // The target that the GPU buffer should be bound to
    };

    struct Accessor
    {
        int bufferViewIndex = -1;                                   // Index of the bufferView
        size_t byteOffset = 0;                                      // Offset into the bufferView in bytes
        GLTFComponentType componentType = GLTFComponentType::FLOAT; // Data type of components in the accessor
        size_t count = 0;                                           // Number of elements or vertices
        GLTFAccessorType type = GLTFAccessorType::SCALAR;           // Type of the element (e.g., VEC3, SCALAR)
        bool normalized = false;                                    // Whether integer data values should be normalized

        // Optional: min and max values, used for bounding volume calculations
        std::vector<float> minValues;
        std::vector<float> maxValues;
    };

    struct MeshPrimitive
    {
        int indices = -1;                            // Index of the accessor for indices (optional)
        int material = -1;                           // Index of the material to use (optional)
        GLTFMeshMode mode = GLTFMeshMode::TRIANGLES; // The type of primitive to render
        std::map<std::string, int> attributes;       // A dictionary object of accessor indices for each vertex attribute
    };

    struct Mesh
    {
        std::string name;                      // Optional name of the mesh
        std::vector<MeshPrimitive> primitives; // Collection of primitives to be rendered
    };

    struct Buffer
    {
        std::string uri;       // URI of the buffer data (could be a path to a file or a data URI)
        size_t byteLength = 0; // Size of the buffer in bytes

        // If you plan to load the buffer data directly into this struct:
        std::vector<char> data; // The actual data of the buffer
    };

    struct Asset
    {
        std::string version;    // Required - the version of the GLTF specification
        std::string generator;  // Optional - the tool that generated the GLTF file
        std::string minVersion; // Optional - the minimum GLTF version that this file is compatible with
    };

    enum class TextureType
    {
        BASE_COLOR,         // For base color of the material
        NORMAL_MAP,         // For normal maps
        METALLIC_ROUGHNESS, // For metallic-roughness workflow
        OCCLUSION,          // For ambient occlusion
        EMISSIVE            // For emissive textures

    };

    struct TextureInfo
    {
        int index = -1;   // Index of the texture in the textures array
        int texCoord = 0; // Texture coordinate set to use

    };

    struct Material
    {
        struct PBRMetallicRoughness
        {
            std::optional<TextureInfo> baseColorTexture;                     // Optional base color texture
            std::array<float, 4> baseColorFactor = {1.0f, 1.0f, 1.0f, 1.0f}; // RGBA base color factor
            std::optional<TextureInfo> metallicRoughnessTexture;             // Optional metallic-roughness texture
            float metallicFactor = 1.0f;                                     // Metallic factor
            float roughnessFactor = 1.0f;                                    // Roughness factor

            PBRMetallicRoughness() = default;

            // Additional constructors or methods can be added as needed
        };

        std::string name;                            // Optional name of the material
        PBRMetallicRoughness pbrMetallicRoughness;   // PBR Metallic-Roughness properties
        std::optional<TextureInfo> normalTexture;    // Optional normal map texture
        std::optional<TextureInfo> occlusionTexture; // Optional occlusion texture
        std::optional<TextureInfo> emissiveTexture;  // Optional emissive texture
    };

    struct Texture
    {
        int source = -1;  // Index of the image used by this texture
        int sampler = -1; // Index of the sampler used by this texture
    };

    struct Sampler
    {
        int magFilter = -1; // Magnification filter
        int minFilter = -1; // Minification filter
        int wrapS = 10497;  // s-coordinate wrapping mode, default is REPEAT
        int wrapT = 10497;  // t-coordinate wrapping mode, default is REPEAT
    };

    struct Scene
    {
        std::string name;             // Optional name of the scene
        std::vector<int> nodeIndices; // Indices of root nodes in the scene
    };

    struct Node
    {
        std::string name;                                             // Optional name of the node
        Eigen::Affine3f transform = Eigen::Affine3f::Identity();      // Affine transformation matrix
        Eigen::Vector3f translation = Eigen::Vector3f::Zero();        // Translation
        Eigen::Quaternionf rotation = Eigen::Quaternionf::Identity(); // Rotation
        Eigen::Vector3f scale = Eigen::Vector3f::Ones();              // Scale

        std::optional<int> mesh;   // Optional index of the mesh in this node
        std::vector<int> children; // Indices of child nodes

        void updateTransform()
        {
            // Construct the transformation matrix from translation, rotation, and scale
            transform = Eigen::Translation3f (translation) * rotation * Eigen::Scaling (scale);
        }
    };

    std::string getFullPathToBinary (const std::string& gltfFilePath, const std::string& bufferUri)
    {
        // Check if the URI is a data URI (base64 encoded)
        if (bufferUri.find ("data:") == 0)
        {
            // Handle data URI (base64 encoded)
            // This might involve decoding the base64 data into a buffer
            return ""; // Returning an empty string as data URIs do not have a file path
        }

        // Construct the full path
        std::filesystem::path gltfPath (gltfFilePath);
        std::filesystem::path dirPath = gltfPath.parent_path();
        std::filesystem::path fullPath = dirPath / bufferUri;

        return fullPath.string();
    }

} // namespace

struct GLTFData
{
    Asset asset;
    std::vector<char> binaryData;
    std::vector<Accessor> accessors;
    std::vector<BufferView> bufferViews;
    std::vector<Mesh> meshes;
    std::vector<Buffer> buffers;
    std::vector<Scene> scenes;
    std::vector<Node> nodes;
    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Sampler> samplers;
};