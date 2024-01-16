#include "Jahley.h"

#include "GLTFParser.h"
#include "GLTFWriter.h"
#include "ModelBuilder.h"

const std::string APP_NAME = "cppGltf";

class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        try
        {
            // get the resource folder for this project
            std::string resourceFolder = getResourcePath (APP_NAME);
            LOG (INFO) << resourceFolder;

            std::string triPath = resourceFolder + "/tri/tri.gltf";
            std::string cubePath = resourceFolder + "/cube/cube.gltf";
         
            //std::string TEMP = "E:/common_content/glTF-Sample-Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf";

            GLTFParser parser (cubePath);
            parser.parse();
            parser.gltfStatistics();
            
            ModelBuilder builder (parser.getData());
            cgModelPtr cgModel = builder.createCgModel();

        }
        catch (std::exception& e)
        {
            LOG(CRITICAL) << e.what();
        }
    }

    ~Application()
    {
    }

    void onCrash() override
    {
    }

 private:
};

Jahley::App* Jahley::CreateApplication()
{
    return new Application();
}
