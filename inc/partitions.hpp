#include "point.hpp"
#include "Lpoint.hpp"

/**
 * @brief Naïve partitioning, only taking into account the point cloud's bounding box
 * and the number of MPI processes
 * @return Vector of min max coordinates pairs
 */
std::vector<std::pair<Point, Point>> naivePart(std::pair<Point, Point> boundingBox, int npes);

/**
 * @brief Partitioning splitting bounding box in npes * npes cells and returning those not empty
 * @return Vector of min max coordinates pairs, probably greater than npes
 */
std::vector<std::pair<Point, Point>> cellPart(std::pair<Point, Point> boundingBox, int npes, const std::vector<Lpoint>& points);

/**
 * @brief Partitioning using decimated point cloud and a quadtree
 * @return Vector of min max coordinates pairs corresponding to filled quadrants
 */
std::vector<std::pair<Point, Point>> quadPart(std::pair<Point, Point> boundingBox, int npes, std::vector<Lpoint>& points);