#pragma once

#include "GLTFUtil.h"

class GLTFParser
{
 public:
    explicit GLTFParser (const std::string& filepath);
    void parse();

    const GLTFData& getData() const { return data; }

 private:
    std::string filepath_;
    json jsonData;

    GLTFData data;

    void loadBinaryFile (const std::string& filename);

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