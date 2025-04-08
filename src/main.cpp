#include "handlers.hpp"
#include "main_options.hpp"
#include "octree.hpp"
#include <filesystem> // Only C++17 and beyond
#include <iomanip>
#include <iostream>
#include <fstream>
#include "TimeWatcher.hpp"
#include <cmath>
#include "decimation.hpp"
#include "cheesemap/cheesemap.hpp"

namespace fs = std::filesystem;

Lpoint centroid(const std::vector<Lpoint>& points);

arma::mat cov(const std::vector<Lpoint>& points);

void features(const std::vector<Lpoint>& neigh, Lpoint& p);

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
		decimateAndRotate(points, mainOptions.dec);

		fs::path outputFile = mainOptions.outputDirName / (fileName + ".las");
		tw.start();
		writePointCloud(outputFile, points);
		tw.stop();
		std::cout << "Time to write point cloud: " << tw.getElapsedDecimalSeconds() << " seconds\n";
	}

	if (mainOptions.radius)
	{
		// cheesemap
		std::cout << "Building global cheesemap..." << std::endl;
		tw.start();
		const auto flags = chs::flags::build::PARALLEL | chs::flags::build::SHRINK_TO_FIT;
		auto map = chs::Dense<Lpoint, 2>(points, mainOptions.cellSize, flags);
		tw.stop();
		std::cout << "Time to build global cheesemap: " << tw.getElapsedDecimalSeconds() << " seconds\n";

		const auto bytes = map.mem_footprint();
		const auto mb    = bytes / (1024.0 * 1024.0);
		std::cout << "Estimated mem. footprint: " << map.mem_footprint() << " Bytes (" << mb << "MB)" << '\n';

		std::cout << "Number of cells: " << map.get_num_cells() << ", of which, empty: " << map.get_empty_cells() << "\n";

		// neigh search
		const float rad = mainOptions.radius;	// search radius
		tw.start();
		#pragma omp parallel for
		for (auto& p : points)
		{
			chs::kernels::Sphere<3> search(p, rad);
			const auto results_map = map.query(search);	// vector with neighs of P inside a sphere of radius rads
			std::vector<Lpoint> neigh{};	// quick conversion to Lpoint vector
			for (auto m : results_map) { neigh.push_back(Lpoint(m[0][0], m[0][1], m[0][2])); }
			features(neigh, p);
		}
		tw.stop();
		std::cout << "Time to calculate descriptors: " << tw.getElapsedDecimalSeconds() << " seconds\n";

		fs::path outputFile = mainOptions.outputDirName / (fileName + "_feat.las");
		tw.start();
		writePointCloudDescriptors(outputFile, points);
		tw.stop();
		std::cout << "Time to write point cloud descriptors: " << tw.getElapsedDecimalSeconds() << " seconds\n";

		// Global Octree Creation
		/*
		std::cout << "Building global octree..." << std::endl;
		tw.start();
		Octree gOctree(points);
		tw.stop();
		std::cout << "Time to build global octree: " << tw.getElapsedDecimalSeconds() << " seconds\n";

		avg = 0;
		tw.start();
		#pragma omp parallel for reduction(+:avg)
		for (const auto& p: points)
		{
			const auto neigh = gOctree.searchNeighbors<Kernel_t::sphere>(p, rad);
			avg += neigh.size();
		}
		tw.stop();
		std::cout << "Average neighbors: " << static_cast<double>(avg) / static_cast<double>(points.size()) <<
					" found in " << tw.getElapsedDecimalSeconds() << " seconds\n";
		*/
	}
	return EXIT_SUCCESS;
}

Lpoint centroid(const std::vector<Lpoint>& points)
{
    double x = 0, y = 0, z = 0;
    for (const auto p : points)
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
    for (const auto p : points)
    {
        // arma::vec v = { p.getX() - c.getX(), p.getY() - c.getY(), p.getZ() - c.getZ() };
		arma::vec v = p - c;
        cov += v * v.t();
    }
    return cov / points.size();
}

void features(const std::vector<Lpoint>& neigh, Lpoint& p)
{
	auto A = cov(neigh);
	arma::vec eigval;
	arma::mat eigvec;
	arma::eig_sym(eigval, eigvec, A);
	arma::vec z = { 0, 0, 1 };

	p.nNeigh = neigh.size();
	p.sum = accu(eigval);													// number of neighbors
	p.omnivar = pow(fabs((eigval[0] * eigval[1] * eigval[2])), 1.0 / 3.0 );	// omnivariance
	p.eigenen = -accu(eigval % arma::log(eigval));							// eigenentropy
	p.linear = (eigval[2] - eigval[1]) / eigval[2];                         // linearity
	p.planar = (eigval[1] - eigval[0]) / eigval[2];                         // planarity
	p.spheric = eigval[0] / eigval[2];                                      // sphericity
	p.curvChange = eigval[0] / (eigval[0] + eigval[1] + eigval[2]);         // change of curvature
	// verticality
	p.vert[0] = fabs(M_PI / 2 - acos(arma::dot(eigvec.col(0), z) / sqrt(arma::dot(eigvec.col(0), eigvec.col(0)) * arma::dot(z, z))));
	p.vert[1] = fabs(M_PI / 2 - acos(arma::dot(eigvec.col(2), z) / sqrt(arma::dot(eigvec.col(2), eigvec.col(2)) * arma::dot(z, z))));
	// absolute moment 1 - 6, vertical moment 1 - 2
	// https://isprs-annals.copernicus.org/articles/III-3/177/2016/isprs-annals-III-3-177-2016.pdf
	std::vector<double> abs(6);
	std::vector<double> vert(2);
	for (const auto n : neigh)
	{
		arma::vec v = n - p;
		abs[0] += arma::dot(v, eigvec.col(0));
		abs[1] += pow(arma::dot(v, eigvec.col(0)), 2);
		abs[2] += arma::dot(v, eigvec.col(1));
		abs[3] += pow(arma::dot(v, eigvec.col(1)), 2);
		abs[4] += arma::dot(v, eigvec.col(2));
		abs[5] += pow(arma::dot(v, eigvec.col(2)), 2);
		vert[0] += arma::dot(v, z);
		vert[1] += pow(arma::dot(v, z), 2);
	}	
	double neighInv = 1.0 / neigh.size();
	p.absMom[0] = neighInv * fabs(abs[0]);
	p.absMom[1] = neighInv * fabs(abs[1]);
	p.absMom[2] = neighInv * fabs(abs[2]);
	p.absMom[3] = neighInv * fabs(abs[3]);
	p.absMom[4] = neighInv * fabs(abs[4]);
	p.absMom[5] = neighInv * fabs(abs[5]);
	p.vertMom[0] = neighInv * vert[0];
	p.vertMom[1] = neighInv * vert[1];
}