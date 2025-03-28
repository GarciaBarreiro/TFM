#include "FileWriterFactory.hpp"

File_t chooseWriterType(const std::string& fExt)
{
    if (fExt == ".las" || fExt == ".laz") return las_t;
    if (fExt == ".txt" || fExt == ".xyz") return txt_t;

    return err_t;
}