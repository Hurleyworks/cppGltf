#include "Jahley.h"
#include <BSthread/BS_thread_pool.hpp>
#include "GLTFParser.h"
#include "GLTFWriter.h"
#include "ModelBuilder.h"

const std::string APP_NAME = "cppGltf";

PathList modelPaths;

void processPath (const std::filesystem::path& p)
{
    if (!std::filesystem::exists (p))
        throw std::runtime_error ("file does not exist: " + p.string());

    if (std::filesystem::is_directory (p))
    {
        for (const auto& entry : std::filesystem::directory_iterator (p))
        {
            processPath (entry.path());
        }
    }
    else
    {
        std::string fileExtension = p.extension().string();

        bool validPath = true;

        // Check if the extension is .gltf or .GLTF
        if (p.extension() != ".gltf" && p.extension() != ".GLTF")
        {
            validPath = false;
        }

        // Check for specific invalid path conditions
        if (!isValidPath (p, "Draco") ||
            !isValidPath (p, "KTX") ||
            !isValidPath (p, "JPG") ||
            // embedded binary not supported yet
            !isValidPath (p, "EMBEDDED") ||
            !isValidPath (p, "Unicode"))
        {
            validPath = false;
        }

        // Add to modelPaths only if all conditions are met
        if (validPath)
        {
            modelPaths.push_back (p);
        }
    }
}

class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        BS::thread_pool pool;

        try
        {
            // get the resource folder for this project
            std::string resourceFolder = getResourcePath (APP_NAME);
            LOG (INFO) << resourceFolder;

            std::string triPath = resourceFolder + "/tri/tri.gltf";
            std::string cubePath = resourceFolder + "/cube/cube.gltf";

            GLTFParser parser (cubePath);
            parser.parse();
            parser.gltfStatistics();

            ModelBuilder builder (parser.getData());
            cgModelPtr cgModel = builder.createCgModel();

            // you can download the 2.0 sample models
            // https://github.com/KhronosGroup/glTF-Sample-Models

            // replace this with your own path
            std::string modelsFolder = "E:/common_content/glTF-Sample-Models";
            processPath (modelsFolder);

            // parse them concurrently in a thread pool
            pool.detach_blocks (0u, (uint32_t)modelPaths.size(),
                                [&] (const uint32_t start, const uint32_t end)
                                {
                                    for (uint32_t i = start; i < end; ++i)
                                    {
                                        const std::filesystem::path& path = modelPaths[i];

                                        try
                                        {
                                            GLTFParser parser (path.generic_string());
                                            bool success = parser.parse();
                                            if (success)
                                            {
                                                ModelBuilder builder (parser.getData());
                                                cgModelPtr cgModel = builder.createCgModel();
                                                if (!cgModel)
                                                {
                                                    LOG (CRITICAL) << "Failed to create cgModel";
                                                }
                                            }
                                        }
                                        catch (std::exception& e)
                                        {
                                            LOG (CRITICAL) << e.what();
                                        }
                                    }
                                });

            pool.wait();

            LOG (DBUG) << "read " << modelPaths.size() << " gltf files";
        }
        catch (std::exception& e)
        {
            pool.wait();
            LOG (CRITICAL) << e.what();
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
