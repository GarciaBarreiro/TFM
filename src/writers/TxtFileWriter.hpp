#pragma once

#include "FileWriter.hpp"
#include "Lpoint.hpp"

class TxtFileWriter : public FileWriter
{
    public:
    TxtFileWriter(const fs::path& path) : FileWriter(path){};
    ~TxtFileWriter(){};

    /**
     * @brief Writes the points to a .txt/.xyz file
     */
    void write(std::vector<Lpoint>& points);
};