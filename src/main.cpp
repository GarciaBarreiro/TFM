#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include "TimeWatcher.hpp"
#include <armadillo>	// find covariance matrix
#include <cmath>

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
	fs::path outputTxt = mainOptions.outputDirName / (fileName + ".txt");
	fs::path outputLas = mainOptions.outputDirName / (fileName + ".las");
	
	// Print three decimals
	std::cout << std::fixed;
	std::cout << std::setprecision(3);

	TimeWatcher tw;

	// TODO: decimate point cloud when reading
	// decimate only when point cloud is big, no need for smaller ones
	// (unless PCA takes more time than expected)
	tw.start();
	std::vector<Lpoint> points = readPointCloud(mainOptions.inputFile);	// TODO: decimate
	tw.stop();
	std::cout << "Number of read points: " << points.size() << "\n";
	std::cout << "Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

	// PCA
	auto A = cov(points);
	arma::vec eigval;
	arma::mat eigvec;
	arma::eig_sym(eigval, eigvec, A);

	// find max eigval and corresponding eigvec (TODO: clean)
	float max = eigval[0];
	int idx = 0;
	for (int i = 1; i < 3; i++) {
		if (abs(eigval[i]) > max) {
			idx = i;
		}
	}

	// normalise eigenvector
	arma::vec unit = arma::normalise(eigvec.col(idx));

	// rotate unitary eigenvec to (1,0,0)
	arma::vec axis {1,0,0};	// column vector

	// cmath acos (arma::acos returns error)
	auto angle = acos(arma::norm_dot(eigvec.col(idx), axis));

	// calc rotation matrix
	arma::mat33 R = arma::eye(3,3) - ((unit + axis)/(1 + arma::dot(unit, axis)))
					* arma::trans(unit + axis) + 2 * axis * arma::trans(unit);

	// apply rotation to all points in point cloud and write point cloud (txt for now)
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

	writePointCloud(outputTxt, points);
	writePointCloud(outputLas, points);

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