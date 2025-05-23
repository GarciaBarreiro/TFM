//
// Created by miguelyermo on 6/8/21.
//

#include "TxtFileReader.hpp"
#include "main_options.hpp"
#include <fstream>
#include <iterator>

std::vector<std::string> splitLine(std::string& line)
{
	std::istringstream                 buf(line);
	std::istream_iterator<std::string> beg(buf), end;
	std::vector<std::string>           tokens(beg, end);

	return tokens;
}

void TxtFileReader::setNumberOfColumns(std::ifstream& file)
{
	std::string line, item;

	std::getline(file, line);
	file.seekg(0); // Return to first line to be read later.
	std::stringstream ss(line);

	numCols = 0;
	while (ss >> item)
		numCols++;
}

std::vector<Lpoint> TxtFileReader::read()
{
	std::ifstream file(path.string());
	std::string   line{};

	setNumberOfColumns(file);

	unsigned int        idx = 0;
	std::vector<Lpoint> points;

	// TODO: Pensar como modularizarlo...
	// TODO: Factory as a function of the number of columns to read different inputs!
	switch (numCols)
	{
		case 3:
			while (std::getline(file, line, '\n'))
			{
				auto tokens = splitLine(line);
				points.emplace_back(idx++,                 // id
				                    std::stod(tokens[0]),  // x
				                    std::stod(tokens[1]),  // y
				                    std::stod(tokens[2])); // z
			}
			break;

		case 7:
			while (std::getline(file, line, '\n'))
			{
				auto tokens = splitLine(line);
				points.emplace_back(idx++,                 // id
				                    std::stod(tokens[0]),  // x
				                    std::stod(tokens[1]),  // y
				                    std::stod(tokens[2]),  // z
				                    std::stod(tokens[3]),  // I
				                    std::stod(tokens[4]),  // rn
				                    std::stod(tokens[5]),  // nor
				                    std::stoi(tokens[6])); // classification
			}
			break;

		// Raw point cloud without RGB
		case 9:
			while (std::getline(file, line, '\n'))
			{
				auto tokens = splitLine(line);
				points.emplace_back(idx++,                 // id
				                    std::stod(tokens[0]),  // x
				                    std::stod(tokens[1]),  // y
				                    std::stod(tokens[2]),  // z
				                    std::stod(tokens[3]),  // I
				                    std::stoi(tokens[4]),  // rn
				                    std::stoi(tokens[5]),  // nor
				                    std::stoi(tokens[6]),  // dir
				                    std::stoi(tokens[7]),  // edge
				                    std::stoi(tokens[8])); // classification
			}
			break;

		case 12:
			while (std::getline(file, line, '\n'))
			{
				auto tokens = splitLine(line);
				points.emplace_back(idx++,                  // id
				                    std::stod(tokens[0]),   // x
				                    std::stod(tokens[1]),   // y
				                    std::stod(tokens[2]),   // z
				                    std::stod(tokens[3]),   // I
				                    std::stoi(tokens[4]),   // rn
				                    std::stoi(tokens[5]),   // nor
				                    std::stoi(tokens[6]),   // dir
				                    std::stoi(tokens[7]),   // edge
				                    std::stoi(tokens[8]),   // classification
				                    std::stoi(tokens[9]),   // r
				                    std::stoi(tokens[10]),  // g
				                    std::stoi(tokens[11])); // b
			}
			break;

		default:
			std::cout << "Unrecognized format\n";
			exit(1);
	}
	file.close();
	std::cout << "Read points: " << idx << "\n";
	return points;
};

std::vector<Lpoint> TxtFileReader::decRead(int jump, float percent)
{
}

std::vector<Lpoint> TxtFileReader::readOverlap(const Box& box, const Box& overlap)
{
}

std::vector<std::vector<Lpoint>> TxtFileReader::readOverlap(const std::vector<Box>& boxes, const std::vector<Box>& overlaps)
{
}

std::pair<Point, Point> TxtFileReader::readBoundingBox()
{
	double x_min = __DBL_MAX__, y_min = __DBL_MAX__, z_min = __DBL_MAX__;
	double x_max = -__DBL_MAX__, y_max = -__DBL_MAX__, z_max = -__DBL_MAX__;
	std::ifstream file(path.string());
	std::string   line{};

	while (std::getline(file, line, '\n'))
	{
		auto tokens = splitLine(line);
		if (std::stod(tokens[0]) < x_min) x_min = std::stod(tokens[0]);
		if (std::stod(tokens[1]) < y_min) y_min = std::stod(tokens[1]);
		if (std::stod(tokens[2]) < z_min) z_min = std::stod(tokens[2]);
		if (std::stod(tokens[0]) > x_max) x_max = std::stod(tokens[0]);
		if (std::stod(tokens[1]) > y_max) y_max = std::stod(tokens[1]);
		if (std::stod(tokens[2]) > z_max) z_max = std::stod(tokens[2]);
	}

	return std::make_pair(Point{x_min, y_min, z_min}, Point{x_max, y_max, z_max});
}