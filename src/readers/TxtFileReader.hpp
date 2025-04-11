//
// Created by miguelyermo on 6/8/21.
//

#pragma once

#include "FileReader.hpp"
/**
 * @brief Specialization of FileRead to read .txt/.xyz files
 */
class TxtFileReader : public FileReader
{
	public:
	uint8_t numCols{};

	// ***  CONSTRUCTION / DESTRUCTION  *** //
	// ************************************ //
	TxtFileReader(const fs::path& path) : FileReader(path){};
	~TxtFileReader(){};

	/**
	 * @brief Reads the points contained in the .txt/.xyz file
	 * @return Vector of Lpoint
	 */
	std::vector<Lpoint> read();

	/**
	 * @brief Sets the number of columns of the file to be read
	 * @return Number of columns of the file
	 */
	void setNumberOfColumns(std::ifstream& file);

	/**
	 * @brief Reads the points contained in the .txt/.xyz file
	 * @return Vector of Lpoint
	 */
	[[deprecated("not yet implemented")]] std::vector<Lpoint> readOverlap(const Box& box, const Box& overlap);

	/**
	 * @brief Reads the bounding box of the .txt/.xyz file
	 * @return Pair of min and max coordinates
	 */
	std::pair<Point, Point> readBoundingBox();
};

std::vector<std::string> splitLine(std::string& line);