#pragma once

#include "GLTFUtil.h"

class GLTFParser
{
 public:
    explicit GLTFParser (const std::string& filepath);
    bool parse();

    const GLTFData& getData() const { return data; }
    void gltfStatistics();

 private:
    std::string filepath_;
    json jsonData;
   
    GLTFData data;

    bool hasBinaryData = false;
    bool loadBinaryFile (const std::string& filename, Buffer& buffer);

    void parseNodes();
    void parseMeshes();
    void parseMaterials();
    void parseAccessors();
    void parseBufferViews();
    void parseBuffers();
    void parseImages();
    void parseScenes();
    void parseAsset();
    void parseTextures();
    void parseSamplers();
};