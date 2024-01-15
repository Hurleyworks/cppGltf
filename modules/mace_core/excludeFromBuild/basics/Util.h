#pragma once

// NO MORE TRACING  Yay!
#define TRACE(x)

const LEVELS TESTING{INFO.value + 1, "TESTING"};
const LEVELS CRITICAL{WARNING.value + 1, "CRTICAL"};

#if defined(_WIN32) || defined(_WIN64)
#define __FUNCTION_NAME__ __func__
#define _FN_ __FUNCTION_NAME__
#ifndef NOMINMAX
#define NOMINMAX
#endif

#undef near
#undef far
#undef RGB
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif


// from the Code Blacksmith
// https://www.youtube.com/watch?v=GV0JMHOpoEw
class ScopedStopWatch
{
 public:
    using Clock = std::conditional_t<std::chrono::high_resolution_clock::is_steady,
                                     std::chrono::high_resolution_clock,
                                     std::chrono::steady_clock>;
    ScopedStopWatch (const char function[] = "unknown function") :
        func (function)
    {
    }
    ~ScopedStopWatch()
    {
        LOG (DBUG) << "\n"
                   << func << " took " << std::chrono::duration_cast<std::chrono::milliseconds> (Clock::now() - start).count() << " milliseconds";
    }

 private:
    const char* func = nullptr;
    Clock::time_point start = Clock::now();
};

// store and retrieve any type from a map
template <class PROPERTY>
class AnyValue
{
    using ValueMap = std::unordered_map<int, std::any>;

 public:
    AnyValue() = default;
    ~AnyValue() = default;

    void addDefault (const PROPERTY& key, const std::any& value) { map_.insert (std::make_pair (key, value)); }
    void setValue (const PROPERTY& key, const std::any& value)
    {
        auto it = map_.find (key);
        if (it == map_.end())
            map_.insert (std::make_pair (key, value));
        else
            it->second = value;
    }

    template <typename T>
    T& getRef (const PROPERTY& key) { return std::any_cast<T> (getValue (key)); }

    template <typename T>
    T getVal (const PROPERTY& key) { return std::any_cast<T> (getValue (key)); }

    template <typename T>
    T* getPtr (const PROPERTY& key) { return std::any_cast<T> (&getValue (key)); }

 private:
    ValueMap map_;
    std::any empty_;

    std::any& getValue (const PROPERTY& key)
    {
        auto it = map_.find (key);
        if (it != map_.end())
            return it->second;

        return empty_;
    }

}; // end class AnyValue



inline std::string getFileNameWithoutExtension (const std::filesystem::path& filePath)
{
    return filePath.stem().string();
}

inline std::filesystem::path changeFileExtensionToJpeg (const std::filesystem::path& pngPath)
{
    std::filesystem::path newPath = pngPath;
    newPath.replace_extension (".jpeg");
    return newPath;
}

inline bool isValidPath (const std::filesystem::path& filePath, const std::string& rejectWord)
{
    // Convert the file path to a string
    std::string pathStr = filePath.generic_string();

    // Convert the string to lowercase for case-insensitive search
    std::transform (pathStr.begin(), pathStr.end(), pathStr.begin(),
                    [] (unsigned char c)
                    { return std::tolower (c); });

    // Convert the reject word to lowercase for case-insensitive comparison
    std::string lowerRejectWord = rejectWord;
    std::transform (lowerRejectWord.begin(), lowerRejectWord.end(), lowerRejectWord.begin(),
                    [] (unsigned char c)
                    { return std::tolower (c); });

    // Search for the reject word in the path string
    if (pathStr.find (lowerRejectWord) != std::string::npos)
    {
        // Reject word is found in the path
        return false;
    }

    // Reject word is not found in the path
    return true;
}

inline bool pathContainsIgnoreCase (const std::filesystem::path& filePath, const std::string& searchWord)
{
    // Convert the file path to a string
    std::string pathStr = filePath.generic_string();

    // Convert the string to lowercase for case-insensitive search
    std::transform (pathStr.begin(), pathStr.end(), pathStr.begin(),
                    [] (unsigned char c)
                    { return std::tolower (c); });

    // Convert the search word to lowercase for case-insensitive comparison
    std::string lowerSearchWord = searchWord;
    std::transform (lowerSearchWord.begin(), lowerSearchWord.end(), lowerSearchWord.begin(),
                    [] (unsigned char c)
                    { return std::tolower (c); });

    // Look for the search word in the path string
    if (pathStr.find (lowerSearchWord) != std::string::npos)
    {
        // word is found in the path
        return true;
    }

    // Reject word is not found in the path
    return false;
}


inline std::string readTxtFile (const std::filesystem::path& filepath)
{
    std::ifstream ifs;
    ifs.open (filepath, std::ios::in);
    if (ifs.fail())
        return "";

    std::stringstream sstream;
    sstream << ifs.rdbuf();

    return std::string (sstream.str());
}

inline std::vector<char> readBinaryFile (const std::filesystem::path& filepath)
{
    std::vector<char> ret;

    std::ifstream ifs;
    ifs.open (filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (ifs.fail())
        return std::move (ret);

    std::streamsize fileSize = ifs.tellg();
    ifs.seekg (0, std::ios::beg);

    ret.resize (fileSize);
    ifs.read (ret.data(), fileSize);

    return std::move (ret);
}

struct FileServices
{
    static void copyFiles (const std::string& searchFolder, const std::string& destFolder, const std::string& extension, bool recursive = true)
    {
        std::filesystem::recursive_directory_iterator dirIt (searchFolder), end;
        while (dirIt != end)
        {
            if (dirIt->path().extension() == extension || extension == "*")
            {
                std::filesystem::copy (dirIt->path(), destFolder + "/" + dirIt->path().filename().string());
            }
            ++dirIt;
        }
    }

    static void moveFiles (const std::string& searchFolder, const std::string& destFolder, const std::string& extension)
    {
        for (const auto& entry : std::filesystem::directory_iterator (searchFolder))
        {
            if (entry.path().extension() == extension || extension == "*")
            {
                std::filesystem::rename (entry, destFolder + "/" + entry.path().filename().string());
            }
        }
    }

    static std::vector<std::filesystem::path> findFilesWithExtension (const std::filesystem::path& searchFolder, const std::string extension)
    {
        std::vector<std::filesystem::path> matchingFiles;

        for (auto const& dir_entry : std::filesystem::directory_iterator{searchFolder})
        {
            if (dir_entry.path().extension().string() == extension)
            {
                matchingFiles.push_back (dir_entry.path());
            }
        }

        return matchingFiles;
    }

    static std::vector<std::string> getFiles (const std::filesystem::path& searchFolder, const std::string& extension, bool recursive)
    {
        std::vector<std::string> files;
        if (recursive)
        {
            for (auto it = std::filesystem::recursive_directory_iterator (searchFolder);
                 it != std::filesystem::recursive_directory_iterator(); ++it)
            {
                
                try
                {
                    if (it->path().extension() == extension)
                    {
                       // LOG (DBUG) << it->path().generic_string();
                        files.push_back (it->path().generic_string());
                    }
                }
                catch (const std::exception& e)
                {
                    // Handle the error, log it, or ignore it.
                    LOG (DBUG) << "Error reading file path: " << e.what();
                }
            }
        }
        else
        {
            for (auto it = std::filesystem::directory_iterator (searchFolder);
                 it != std::filesystem::directory_iterator(); ++it)
            {
                try
                {
                    LOG (DBUG) << it->path().generic_string();
                    if (it->path().extension() == extension)
                    {
                        files.push_back (it->path().generic_string());
                    }
                }
                catch (const std::exception& e)
                {
                    // Handle the error, log it, or ignore it.
                    LOG (DBUG) << "Error reading file path: " << e.what();
                }
            }
        }
        return files;
    }


    static std::vector<std::string> getFolders (const std::string& searchFolder, bool recursive = true)
    {
        std::vector<std::string> folders;
        std::filesystem::recursive_directory_iterator dirIt (searchFolder), end;
        while (dirIt != end)
        {
            if (std::filesystem::is_directory (dirIt->status()))
            {
                folders.push_back (dirIt->path().string());
            }
            ++dirIt;
        }
        return folders;
    }

    static std::vector<std::string> getTextFileLines (const std::string& filePath)
    {
        std::vector<std::string> lines;
        std::ifstream file (filePath);
        if (!file)
            return lines;

        std::string line;
        while (getline (file, line))
        {
            lines.push_back (line);
        }
        return lines;
    }

    static std::string findFilePath (const std::string& searchFolder, const std::string& fileName)
    {
        std::filesystem::recursive_directory_iterator dirIt (searchFolder), end;
        while (dirIt != end)
        {
            if (dirIt->path().filename() == fileName)
            {
                return dirIt->path().string();
            }
            ++dirIt;
        }
        return "";
    }

    static std::optional<std::filesystem::path> findFileInFolder (
        const std::filesystem::path& folder,
        const std::string& filename)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator (folder))
        {
            if (entry.path().filename() == filename)
            {
                return entry.path();
            }
        }
        return std::nullopt;
    }
};

inline std::string getParentFolderName (const std::filesystem::path& path)
{
    return path.parent_path().filename().string();
}

inline bool hasObjExtension (const std::filesystem::path& filePath)
{
    return filePath.extension() == ".obj";
}

inline bool hasGltfExtension (const std::filesystem::path& filePath)
{
    return filePath.extension() == ".gltf";
   // return filePath.extension() == ".gltf" || filePath.extension() == ".glb";
}

inline bool isStaticBody (const std::filesystem::path& filePath)
{
    std::string filename = filePath.stem().string();

    // Check if filename starts with "static"
    return filename.rfind ("static", 0) == 0;
}

namespace mace
{
    // I think this is from InstantMeshes
    inline bool atomicCompareAndExchange (volatile uint32_t* v, uint32_t newValue, uint32_t oldValue)
    {
#if defined(_WIN32)
        return _InterlockedCompareExchange (
                   reinterpret_cast<volatile long*> (v), (long)newValue, (long)oldValue) == (long)oldValue;
#else
        return __sync_bool_compare_and_swap (v, oldValue, newValue);
#endif
    }

    inline uint32_t atomicAdd (volatile uint32_t* dst, uint32_t delta)
    {
#if defined(_MSC_VER)
        return _InterlockedExchangeAdd (reinterpret_cast<volatile long*> (dst), delta) + delta;
#else
        return __sync_add_and_fetch (dst, delta);
#endif
    }

    inline float atomicAdd (volatile float* dst, float delta)
    {
        union bits
        {
            float f;
            uint32_t i;
        };
        bits oldVal, newVal;
        do
        {
#if defined(__i386__) || defined(__amd64__)
            __asm__ __volatile__ ("pause\n");
#endif
            oldVal.f = *dst;
            newVal.f = oldVal.f + delta;
        } while (!atomicCompareAndExchange ((volatile uint32_t*)dst, newVal.i, oldVal.i));
        return newVal.f;
    }
} // namespace mace