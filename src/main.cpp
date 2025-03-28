#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include <fstream>
#include "TimeWatcher.hpp"
#include <armadillo>	// find covariance matrix
#include <cmath>
#include "decimation.hpp"
#include "Geometry/OBB.h"
#include "Math/float3.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	setDefaults();
	processArgs(argc, argv);

	fs::path    inputFile = mainOptions.inputFile;
	std::string fileName  = inputFile.stem();

	if (!mainOptions.outputDirName.empty()) { mainOptions.outputDirName = mainOptions.outputDirName / fileName; }
	createDirectory(mainOptions.outputDirName);

	// Print three decimals
	std::cout << std::fixed;
	std::cout << std::setprecision(3);

	TimeWatcher tw;

	tw.start();
	std::vector<Lpoint> points = readPointCloud(mainOptions.inputFile);
	tw.stop();
	std::cout << "Number of read points: " << points.size() << "\n";
	std::cout << "Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

	// decimation (only if stated as such)
	if (mainOptions.dec > 0)
	{
		// create OBB using MathGeoLib
		tw.start();
		std::vector<float3> dec_points = jump_dec(points, mainOptions.dec);	// decimate and set Z = 0 for all points
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

		// apply rotation to all points in point cloud and write point cloud
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
		
		fs::path outputFile = mainOptions.outputDirName / (fileName + ".las");
		tw.start();
		writePointCloud(outputFile, points);
		tw.stop();
		std::cout << "Time to write point cloud: " << tw.getElapsedDecimalSeconds() << " seconds\n";
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