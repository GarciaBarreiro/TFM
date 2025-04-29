//
// Created by miguelyermo on 6/8/21.
//

#pragma once

#include "FileReader.hpp"
#include "Lpoint.hpp"
#include "Box.hpp"
#include <lasreader.hpp>

/**
 * @author Miguel Yermo
 * @brief Specialization of FileRead to read .las/.laz files
 */
class LasFileReader : public FileReader
{
	public:
	// ***  CONSTRUCTION / DESTRUCTION  *** //
	// ************************************ //
	LasFileReader(const fs::path& path) : FileReader(path){};
	~LasFileReader(){};

	/**
	 * @brief Reads the points contained in the .las/.laz file
	 * @return Vector of Lpoint
	 */
	std::vector<Lpoint> read();

    /**
     * @brief Reads the points contained in the .las/.laz file that are inside the box and overlap
     * @return Vector of Lpoint
     */
    std::vector<Lpoint> readOverlap(const Box& box, const Box& overlap);

	/**
     * @brief Reads the points contained in the .las/.laz file that are inside boxes and overlaps
     * @return Vector of vectors of Lpoint
     */
	std::vector<std::vector<Lpoint>> readOverlap(const std::vector<Box>& boxes, const std::vector<Box>& overlaps);

	/**
	 * @brief Reads the bounding box of the .las/.laz file
	 * @return Pair of min and max coordinates
	 */
	std::pair<Point, Point> readBoundingBox();
};