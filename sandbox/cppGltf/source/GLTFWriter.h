#pragma once

#include "GLTFUtil.h"

class GLTFWriter
{
 public:
    GLTFWriter() = default;
    
    void write (const std::string& filepath, const GLTFData&& data);
   
 private:
    void writeAccessors (std::ofstream& file, const std::vector<Accessor>& accessors);
    void writeBufferViews (std::ofstream& file, const std::vector<BufferView>& bufferViews);
    void writeBuffers (std::ofstream& file, const std::vector<Buffer>& buffers);
    void writeMeshes (std::ofstream& file, const std::vector<Mesh>& meshes);
    void writeAsset (std::ofstream& file, const Asset& asset);

    void saveBufferData (const Buffer& buffer);
};