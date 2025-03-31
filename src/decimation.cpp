#include "decimation.hpp"
#include <numeric>
#include <algorithm>
#include <random>
#include "TimeWatcher.hpp"
#include "Geometry/OBB.h"
#include "Math/float3.h"
#include <armadillo>

std::vector<float3> jumpDec(const std::vector<Lpoint>& points, int jump)
{
    std::vector<float3> dec;
    for (int i = 0; i < points.size(); i += jump)
    {
        dec.push_back(float3(points[i].getX(), points[i].getY(), 0));
	}
    return dec;
}

std::vector<float3> randomDec(const std::vector<Lpoint>& points, int n)
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
        dec.push_back(float3(p.getX(), p.getY(), 0));
    }
    return dec;
}


// TODO
[[deprecated("not yet implemented")]]
std::vector<float3> gridDec(const std::vector<Lpoint>& points, int cells)
{
    std::vector<float3> dec;

    return dec;
}

void decimateAndRotate(std::vector<Lpoint>& points, int dec)
{
    TimeWatcher tw;
    // create OBB using MathGeoLib
    tw.start();
    std::vector<float3> dec_points = jumpDec(points, dec);	// decimate and set Z = 0 for all points
    // sometimes prints Error: volume > 0.g in OBB.cpp:1942, but seems to work nonetheless
    OBB obb = OBB::OptimalEnclosingOBB(dec_points.data(), dec_points.size());
    tw.stop();
    std::cout << "Time to compute OBB with " << dec_points.size() << " points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

    // one of the axis will always be (0,0,1) or (0,0,-1) (usually [0])
    int idx = (fabs(obb.axis[0][0]) < 0.0001) ? 1 : 0;
    arma::vec unit = {obb.axis[idx][0], obb.axis[idx][1], obb.axis[idx][2]};	// already normalised
    arma::vec axis {1,0,0};
    auto angle = acos(arma::norm_dot(unit, axis));

    // calc rotation matrix
    // https://math.stackexchange.com/questions/4167802/get-a-rotation-matrix-which-rotates-4d-vector-to-another-4d-vector
    arma::mat33 R = arma::eye(3,3) - ((unit + axis)/(1 + arma::dot(unit, axis)))
                    * arma::trans(unit + axis) + 2 * axis * arma::trans(unit);

    tw.start();
    // apply rotation to all points in point cloud and write point cloud
    #pragma omp parallel for
    for (Lpoint& p : points) {
        arma::mat point(3,1);
        point(0) = p.getX();
        point(1) = p.getY();
        point(2) = p.getZ();

        point = R * point;

        p.setX(point(0));
        p.setY(point(1));
        p.setZ(point(2));
    }
    tw.stop();
    std::cout << "Time to apply rotation: " << tw.getElapsedDecimalSeconds() << " seconds\n";
}