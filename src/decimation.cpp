#include "decimation.hpp"
#include <numeric>
#include <algorithm>
#include <random>

std::vector<float3> jump_dec(const std::vector<Lpoint>& points, int jump)
{
    std::vector<float3> dec;
    for (int i = 0; i < points.size(); i += jump)
    {
		// dec.push_back(float3(points[i].getX(), points[i].getY(), points[i].getZ()));
        dec.push_back(float3(points[i].getX(), points[i].getY(), 0));
	}
    return dec;
}

std::vector<float3> random_dec(const std::vector<Lpoint>& points, int n)
{
    std::vector<int> indices(points.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    std::vector<float3> dec;
    for (int i = 0; i < n; i++)
    {
        Lpoint p = points[indices[i]];
        // dec.push_back(float3(p.getX(), p.getY(), p.getZ()));
        dec.push_back(float3(p.getX(), p.getY(), 0));
    }
    return dec;
}


// TODO
[[deprecated("not yet implemented")]]
std::vector<float3> grid_dec(const std::vector<Lpoint>& points, int cells)
{
    std::vector<float3> dec;

    return dec;
}