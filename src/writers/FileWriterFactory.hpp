#pragma once

#include "FileType.hpp"
#include "LasFileWriter.hpp"
#include "TxtFileWriter.hpp"
#include <filesystem>

namespace fs = std::filesystem;

File_t chooseWriterType(const std::string& fExt);

class FileWriterFactory
{
    public:
    static std::shared_ptr<FileWriter> makeWriter(File_t type, const fs::path& path)
    {
        switch (type)
        {
        case txt_t:
            std::cout << "Unimplemented\n";
            exit(-2);
        case las_t:
            return std::make_shared<LasFileWriter>(path);
        default:
            std::cout << "Unable to create specialized FileWriter\n";
            exit(-2);
        }
    }
};