#include "point.hpp"

/**
 * @brief Naïve partitioning, only taking into account the point cloud's bounding box
 * and the number of MPI processes
 * @return Vector of min max coordinates pairs
 */
std::vector<std::pair<Point, Point>> naivePart(std::pair<Point, Point> boundingBox, int npes);