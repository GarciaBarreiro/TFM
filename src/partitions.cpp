#include "partitions.hpp"
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