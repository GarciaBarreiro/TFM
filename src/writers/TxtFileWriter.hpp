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

    /**
     * @brief Writes the points and its corresponding descriptors
     * to a .txt/.xyz file
     */
    void writeDescriptors(std::vector<Lpoint>& points);
};