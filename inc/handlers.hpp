//
// Created by miguelyermo on 1/3/20.
//

/*
* FILENAME :  handlers.h  
* PROJECT  :  rule-based-classifier-cpp
* DESCRIPTION :
*  
*
*
*
*
* AUTHOR :    Miguel Yermo        START DATE : 03:07 1/3/20
*
*/

#ifndef CPP_HANDLERS_H
#define CPP_HANDLERS_H

#include "main_options.hpp"
#include "point.hpp"
#include "Lpoint.hpp"
#include <filesystem> // File extensions
#include <string>
#include <vector>
#include "Box.hpp"

namespace fs = std::filesystem;

void handleNumberOfPoints(std::vector<Lpoint>& points);

unsigned int getNumberOfCols(const fs::path& filePath);

void createDirectory(const fs::path& dirname);

std::vector<Lpoint> readPointCloud(const fs::path& filename);

std::vector<Lpoint> readPointCloudDec(const fs::path& filename, int dec, float percent);

std::vector<Lpoint> readPointCloudOverlap(const fs::path& filename, const Box& box, const Box& overlap);

std::vector<std::vector<Lpoint>> readPointCloudOverlap(const fs::path& filename, const std::vector<Box>& boxes, const std::vector<Box>& overlaps);

std::pair<Point, Point> readBoundingBox(const fs::path& filename);

void writePointCloud(const fs::path& fileName, std::vector<Lpoint>& points);

void writePointCloudDescriptors(const fs::path& fileName, std::vector<Lpoint>& points);

#endif //CPP_HANDLERS_H
