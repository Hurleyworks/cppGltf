#pragma once

#include "GLTFUtil.h"

class GLTFWriter
{
 public:
    GLTFWriter() = default;
    ~GLTFWriter() = default;

    void write (const std::string& filepath, const GLTFData&& data);
   
 private:
    std::string filePath;

    void writeAccessors (std::ofstream& file, const std::vector<Accessor>& accessors);
    void writeBufferViews (std::ofstream& file, const std::vector<BufferView>& bufferViews);
    void writeBuffers (std::ofstream& file, const std::vector<Buffer>& buffers);
    void writeMeshes (std::ofstream& file, const std::vector<Mesh>& meshes);
    void writeAsset (std::ofstream& file, const Asset& asset);
    void writeScenes (std::ofstream& file, const std::vector<Scene>& scenes);
    void writeNodes (std::ofstream& file, const std::vector<Node>& nodes); 
    void writeMaterials (std::ofstream& file, const std::vector<Material>& materials);
    void writeTextures (std::ofstream& file, const std::vector<Texture>& textures);
    void writeSamplers (std::ofstream& file, const std::vector<Sampler>& samplers);
    void writeImages (std::ofstream& file, const std::vector<Image>& images);
    void saveBufferData (const Buffer& buffer);
};