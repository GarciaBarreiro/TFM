#pragma once

#include "Lpoint.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class FileWriter
{
    protected:
    /**
	 * @brief Path to file to be written
	 */
    fs::path path{};

    public:

    /**
	 * @brief Instantiate a FileWriter which writes a file to a given path
	 * @param path
	 */
	FileWriter(const fs::path& path) : path(path){};
	virtual ~FileWriter(){}; // Every specialization of this class must manage its own destruction
    virtual void write(std::vector<Lpoint>& points) = 0;
};