#include "partitions.hpp"
#include "Lpoint.hpp"
#include "decimation.hpp"
#include "main_options.hpp"
#include "Box.hpp"
#include "quadtree.h"
#include <queue>

std::vector<std::pair<Point, Point>> naivePart(std::pair<Point, Point> boundingBox, int npes)
{
    const double epsilon = 0.000001;
    // using queue so first element is always the biggest box
    std::queue<std::pair<Point, Point>> boxes;
    boxes.emplace(boundingBox.first - epsilon, boundingBox.second);

    for (int i = npes - 1; i > 0; i--)
    {
        // get 1st elem and delete it
        auto b = boxes.front();
        boxes.pop();

        // get max length and what axis
        double max_l = std::max({ b.second.getX() - b.first.getX(), b.second.getY() - b.first.getY() });
        char axis = (fabs(max_l - (b.second.getY() - b.first.getY())) < epsilon) ? 'y' : 'x';

        // check if last iter if number of processes is odd
        bool last = i == 2 && npes % 2 == 1;
        double split = (last) ? max_l / 3.0 : max_l / 2.0;
        int mult = (last) ? 2 : 1;

        if (axis == 'x')
        {
            boxes.emplace(b.first, Point(b.first.getX() + split, b.second.getY(), b.second.getZ()));
            if (last)
                boxes.emplace(Point(b.first.getX() + split, b.first.getY(), b.first.getZ()), Point(b.first.getX() + mult * split, b.second.getY(), b.second.getZ()));
            boxes.emplace(Point(b.first.getX() + mult * split, b.first.getY(), b.first.getZ()), b.second);
        } else
        {
            boxes.emplace(b.first, Point(b.second.getX(), b.first.getY() + split, b.second.getZ()));
            if (last)
                boxes.emplace(Point(b.first.getX(), b.first.getY() + split, b.first.getZ()), Point(b.second.getX(), b.first.getY() + mult * split, b.second.getZ()));
            boxes.emplace(Point(b.first.getX(), b.first.getY() + mult * split, b.first.getZ()), b.second);
        }

        if (last) break;
    }

    // convert to vector (contiguous in memory, easier for scatter)
    std::vector<std::pair<Point, Point>> ret;
    while (!boxes.empty())
    {
        ret.emplace_back(boxes.front().first, boxes.front().second + epsilon);
        boxes.pop();
    }

    return ret;
}

std::vector<std::pair<Point, Point>> cellPart(std::pair<Point, Point> boundingBox, int npes, const std::vector<Lpoint>& points)
{
    // get dims, split BB in cells, read point cloud and put points in cells
    // then return only non-empty cells, ordered from most to least points
    auto temp = jumpDec(points, 1000);
    std::vector<Lpoint> decPoints;
    decPoints.reserve(temp.size());
    for (auto t : temp) { decPoints.push_back(Lpoint(t.x, t.y, t.z)); } // probably faster way to convert them
    temp.clear();

    // create 2D grid
    const double xlen = (boundingBox.second.getX() - boundingBox.first.getX()) / npes;
    const double ylen = (boundingBox.second.getY() - boundingBox.first.getY()) / npes;
    std::vector<int> cells(npes * npes, 0);
    #pragma omp parallel for
    for (const auto p : decPoints)
    {
        auto xidx = static_cast<int>(std::floor((p.getX() - boundingBox.first.getX()) / xlen));
        auto yidx = static_cast<int>(std::floor((p.getY() - boundingBox.first.getY()) / ylen));
        #pragma omp critical
        cells[xidx * npes + yidx]++;
    }

    // TODO: order points to get better load balancing
    std::vector<std::pair<Point, Point>> ret;
    for (int i = 0; i < npes * npes; i++)
    {
        if (cells[i] == 0) { continue; }
        Point min{boundingBox.first.getX() + std::floor(i / npes) * xlen,
                boundingBox.first.getY() + (i % npes) * ylen,
                boundingBox.first.getZ()};
        Point max{boundingBox.first.getX() + std::floor((i + npes) / npes) * xlen,
                boundingBox.first.getY() + ((i % npes) + 1) * ylen,
                boundingBox.second.getZ()};
        ret.emplace_back(min, max);
    }

    return ret;
}

std::vector<std::pair<Point, Point>> quadPart(std::pair<Point, Point> boundingBox, int npes, const std::vector<Lpoint>& points)
{
    auto temp = jumpDec(points, 1000);
    std::vector<Lpoint> decPoints;
    decPoints.reserve(temp.size());
    for (auto t : temp) { decPoints.push_back(Lpoint(t.x, t.y, t.z)); }
    temp.clear();

    // create quadtree (smaller quadrants means better load balancing)
    Quadtree quad(decPoints, boundingBox, decPoints.size() / (npes * npes));

    // return quadrants
    // setting correct Z, because the way quadrants are created modifies the original values of Z
    auto ret = quad.getQuadrants();
    for (auto& r : ret)
    {
        r.first.setZ(boundingBox.first.getZ());
        r.second.setZ(boundingBox.second.getZ());
    }
    return ret;
}