#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include "TimeWatcher.hpp"
#include <armadillo>	// find covariance matrix
#include <cmath>
#include "Geometry/OBB.h"
#include "Math/float3.h"
// #include "Geometry/GeometryAll.h"

namespace fs = std::filesystem;

Lpoint centroid(const std::vector<Lpoint>& points)
{
    double x = 0, y = 0, z = 0;
    for (const Lpoint& p : points)
    {
        x += p.getX();
        y += p.getY();
        z += p.getZ();
    }
    return {x / points.size(), y / points.size(), z / points.size()};
}

arma::mat cov(const std::vector<Lpoint>& points)
{
	auto c = centroid(points);

    arma::mat cov(3, 3, arma::fill::zeros);
    for (const Lpoint& p : points)
    {
        arma::vec v = { p.getX() - c.getX(), p.getY() - c.getY(), p.getZ() - c.getZ() };
        cov += v * v.t();
    }
    return cov / points.size();
}

int main(int argc, char* argv[])
{
	setDefaults();
	processArgs(argc, argv);

	fs::path    inputFile = mainOptions.inputFile;
	std::string fileName  = inputFile.stem();

	if (!mainOptions.outputDirName.empty()) { mainOptions.outputDirName = mainOptions.outputDirName / fileName; }
	createDirectory(mainOptions.outputDirName);
	mainOptions.outputDirName = mainOptions.outputDirName / (fileName + ".txt");
	FILE *fp = fopen(mainOptions.outputDirName.c_str(), "w");
	if (!fp) {
		printf("Error opening file %s\n", mainOptions.outputDirName.c_str());
		exit(1);
	}

	// Print three decimals
	std::cout << std::fixed;
	std::cout << std::setprecision(3);

	TimeWatcher tw;

	// TODO: decimate point cloud when reading
	tw.start();
	std::vector<Lpoint> points = readPointCloud(mainOptions.inputFile);	// TODO: decimate
	tw.stop();
	std::cout << "Number of read points: " << points.size() << "\n";
	std::cout << "Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

	// create OBB using MathGeoLib

	// convert std::vector<Lpoint> to math::vec
	std::vector<float3> points_f3;
	for (const Lpoint& p : points) {
		points_f3.push_back(float3(p.getX(), p.getY(), p.getZ()));
	}

	// prints Error: volume > 0.g in OBB.cpp:1942, but seems to work nonetheless
	OBB obb = OBB::OptimalEnclosingOBB(points_f3.data(), points_f3.size());

	std::cout << obb.axis[0] << "\n";
	std::cout << obb.axis[1] << "\n";
	std::cout << obb.axis[2] << "\n";

	// using axis[1] because for alcoy axis[0] is Z
	// but maybe for another point cloud axis[1] is Z, so search for a way to automatize this
	arma::vec unit = {obb.axis[1][0], obb.axis[1][1], obb.axis[1][2]};
	arma::vec axis {1,0,0};
	auto angle = acos(arma::norm_dot(unit, axis));

	// calc rotation matrix
	// https://math.stackexchange.com/questions/4167802/get-a-rotation-matrix-which-rotates-4d-vector-to-another-4d-vector
	arma::mat33 R = arma::eye(3,3) - ((unit + axis)/(1 + arma::dot(unit, axis)))
					* arma::trans(unit + axis) + 2 * axis * arma::trans(unit);

	// apply rotation to all points in point cloud and write point cloud (txt for now)
	for (const Lpoint& p : points) {
		arma::mat point(3,1);
		point(0) = p.getX();
		point(1) = p.getY();
		point(2) = p.getZ();

		point = R * point;

		fprintf(fp, "%lf %lf %lf\n", point(0), point(1), point(2));
	}

	// Global Octree Creation
	/*
	std::cout << "Building global octree..." << std::endl;
	tw.start();
	Octree gOctree(points);
	tw.stop();
	std::cout << "Time to build global octree: " << tw.getElapsedDecimalSeconds() << " seconds\n";
	*/

	return EXIT_SUCCESS;
}