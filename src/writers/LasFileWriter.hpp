#pragma once

#include "FileWriter.hpp"
#include "Lpoint.hpp"
#include <laswriter.hpp>

class LasFileWriter : public FileWriter
{
    public:
    LasFileWriter(const fs::path& path) : FileWriter(path){};
    ~LasFileWriter(){};

    /**
	 * @brief Writes the points to a .las/.laz file
	 */
    void write(std::vector<Lpoint>& points);

    /**
     * @brief Writes the points and its corresponding descriptors
     * to a .las/.laz file
     */
    void writeDescriptors(std::vector<Lpoint>& points);
};