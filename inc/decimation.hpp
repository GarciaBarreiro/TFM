#ifndef CPP_DECIMATION_H
#define CPP_DECIMATION_H

#include "Lpoint.hpp"
#include "Math/float3.h"

// select points 0, jump, 2*jump, 3*jump...
std::vector<float3> jumpDec(const std::vector<Lpoint>& points, int jump);

// shuffle indices from 0 to points.size() and select first n
std::vector<float3> randomDec(const std::vector<Lpoint>& points, int n);

// create a grid of 
std::vector<float3> gridDec(const std::vector<Lpoint>& points, int cells);

// apply decimation and rotate point cloud
void decimateAndRotate(std::vector<Lpoint>& points, int dec);

#endif //CPP_DECIMATION_H