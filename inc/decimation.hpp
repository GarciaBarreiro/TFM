#ifndef CPP_DECIMATION_H
#define CPP_DECIMATION_H

#include "Lpoint.hpp"
#include "Math/float3.h"

// select points 0, jump, 2*jump, 3*jump...
std::vector<float3> jump_dec(const std::vector<Lpoint>& points, int jump);

// shuffle indices from 0 to points.size() and select first n
std::vector<float3> random_dec(const std::vector<Lpoint>& points, int n);

// create a grid of 
std::vector<float3> grid_dec(const std::vector<Lpoint>& points, int cells);

#endif //CPP_DECIMATION_H