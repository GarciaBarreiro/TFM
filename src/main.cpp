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

	double x_std = 0, y_std = 0, z_std = 0;
	for (const Lpoint& p : points) {
		x_std += pow(p.getX() - c.getX(), 2);
		y_std += pow(p.getY() - c.getY(), 2);
		z_std += pow(p.getZ() - c.getZ(), 2);
	}
	x_std = sqrt(x_std/points.size());
	y_std = sqrt(y_std/points.size());
	z_std = sqrt(z_std/points.size());

    arma::mat cov(3, 3, arma::fill::zeros);
    for (const Lpoint& p : points)
    {
        arma::vec v = { (p.getX() - c.getX())/x_std, (p.getY() - c.getY())/y_std, (p.getZ() - c.getZ())/z_std };
        cov += v * v.t();
    }
    return cov / points.size();
}

// TODO: clean
arma::mat rot(const arma::vec& a, float angle)
{
	arma::mat R(3,3);

	auto cosa = cos(angle);
	auto mcosa = 1 - cosa;
	auto sina = sin(angle);

	// (row, column)
	R(0,0) = a(0)*a(0)*mcosa + cosa;
	R(0,1) = a(0)*a(1)*mcosa - a(2)*sina;
	R(0,2) = a(0)*a(2)*mcosa + a(1)*sina;

	R(1,0) = a(1)*a(0)*mcosa + a(2)*sina;
	R(1,1) = a(1)*a(1)*mcosa + cosa;
	R(1,2) = a(1)*a(2)*mcosa - a(0)*sina;

	R(2,0) = a(2)*a(0)*mcosa - a(1)*sina;
	R(2,0) = a(2)*a(1)*mcosa + a(0)*sina;
	R(2,0) = a(2)*a(2)*mcosa + cosa;

	return R;
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

	// find max eigval and corresponding eigvec
	float max = eigval[0];
	int idx = 0;
	for (int i = 1; i < 3; i++) {
		if (eigval[i] > max) {
			idx = i;
		}
	}

	// rotate alongside (1,0,0), (0,1,0) or (0,0,1)
	// TODO: clean
	arma::vec axis {1,0,0};	// axis
	if (idx == 1) axis = {0,1,0};
	else if (idx == 2) axis = {0,0,1};

	// cmath acos (arma::acos returns error)
	auto angle = acos(arma::norm_dot(eigvec.col(idx), axis));

	// calc rotation matrix (TODO: cleaner way?)
	auto R = rot(axis, angle);

	std::cout << A << "\n";
	
	std::cout << eigval << "\n";
	std::cout << eigvec << "\n";

	std::cout << R << "\n";

	// apply rotation to all points in point cloud and write point cloud (txt for now)
	// standardize points????
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