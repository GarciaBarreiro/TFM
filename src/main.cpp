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
#include <mpi.h>
#include "partitions.hpp"
#include "Box.hpp"
#include <fstream>

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

	// init MPI
	int rank = 0, npes = 1;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &npes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	std::vector<Lpoint> points;
	std::vector<std::pair<Point, Point>> boxes;

	if (rank == 0)
	{
		tw.start();
		points = readPointCloud(inputFile);		// TODO: decimate in read, then move this to dec > 0
		tw.stop();
		std::cout << "Number of read points: " << points.size() << "\n";
		std::cout << "Time to read points: " << tw.getElapsedDecimalSeconds() << " seconds\n";

		// decimation (only if stated as such, easier doing it on 1 node)
		if (mainOptions.dec > 0)
		{
			decimateAndRotate(points, mainOptions.dec);

			string ext = (mainOptions.zip) ? ".laz" : ".las";
			fs::path outputFile = mainOptions.outputDirName / (fileName + ext);
			tw.start();
			writePointCloud(outputFile, points);
			tw.stop();
			std::cout << "Time to write point cloud: " << tw.getElapsedDecimalSeconds() << " seconds\n";

			inputFile = outputFile;
		}

		// get point cloud bounding box, split it
		if (mainOptions.radius > 0)
		{
			tw.start();
			auto minmax = readBoundingBox(inputFile);
			// boxes = naivePart(minmax, npes);
			// boxes = cellPart(minmax, npes, points);	// points passed are always not decimated 
			// boxes = cellMergePart(minmax, npes, points);
			boxes = quadPart(minmax, npes, points);
			tw.stop();
			std::cout << "Time to partition point cloud: " << tw.getElapsedDecimalSeconds() << " seconds\n";
		}

		points.clear();
	}

	if (mainOptions.radius > 0)
	{
		std::string debstr;
		debstr += std::to_string(npes) + ", " + std::to_string(rank) + ", ";

		// get sendcounts and displacements for MPI_Scatterv, then send data as MPI_BYTE
		int boxsize = (rank == 0) ? boxes.size() : 0;
		MPI_Bcast(&boxsize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		std::vector<int> sendcounts(npes, std::ceil(static_cast<double>(boxsize)/npes));
		std::vector<int> displs(npes, 0);
		size_t pairsize = sizeof(std::pair<Point, Point>);
		int mod = npes - boxsize % npes;
		int i = npes - 1;
		while (mod && mod != npes) {
			sendcounts[i--]--;
			if (i < 0) i = boxsize - 1;	// shouldn't happen
			mod--;
		}
		for (int i = 0; i < sendcounts.size(); i++)
		{
			sendcounts[i] *= pairsize;
			displs[i] = (i == 0) ? 0 : sendcounts[i-1] + displs[i-1];
		}

		std::vector<std::pair<Point, Point>> lboxes;
		lboxes.resize(sendcounts[rank]/pairsize);
		MPI_Scatterv(boxes.data(), sendcounts.data(), displs.data(), MPI_BYTE, lboxes.data(), sendcounts[rank], MPI_BYTE, 0, MPI_COMM_WORLD);
		lboxes.shrink_to_fit();

		std::vector<Lpoint> totPoints;	// vector to append points to after each iteration
		const float rad = mainOptions.radius;	// search radius
		for (auto minmax : lboxes)
		{
			Box b(minmax);
			Box overlap(std::pair<Point, Point>(minmax.first - rad, minmax.second + rad));

			// read points
			tw.start();
			points = readPointCloudOverlap(inputFile, b, overlap);
			tw.stop();
			int nover = 0;
			#pragma omp parallel for reduction(+:nover)
			for (auto& p : points) { if (p.overlap) nover++; }
			debstr += std::to_string(tw.getElapsedDecimalSeconds()) + ", " +
					std::to_string(points.size()) + ", " + std::to_string(nover) + ", ";

			// cheesemap
			std::cout << "Building global cheesemap..." << std::endl;
			tw.start();
			const auto flags = chs::flags::build::PARALLEL | chs::flags::build::SHRINK_TO_FIT;
			auto map = chs::Dense<Lpoint, 2>(points, mainOptions.cellSize, flags);
			tw.stop();
			std::cout << rank << ": Time to build global cheesemap of " << points.size() << ": " << tw.getElapsedDecimalSeconds() << " seconds\n";

			const auto bytes = map.mem_footprint();
			const auto mb    = bytes / (1024.0 * 1024.0);
			std::cout << "Estimated mem. footprint: " << map.mem_footprint() << " Bytes (" << mb << "MB)" << '\n';

			std::cout << "Number of cells: " << map.get_num_cells() << ", of which, empty: " << map.get_empty_cells() << "\n";
			debstr += std::to_string(tw.getElapsedDecimalSeconds()) + ", " +
					std::to_string(map.get_num_cells()) + ", " + std::to_string(map.get_empty_cells()) + ", ";

			// neigh search
			tw.start();
			#pragma omp parallel for
			for (auto& p : points)
			{
				if (p.overlap) continue;
				chs::kernels::Sphere<3> search(p, rad);
				const auto results_map = map.query(search);	// vector with neighs of P inside a sphere of radius rads
				std::vector<Lpoint> neigh{};	// quick conversion to Lpoint vector
				for (auto m : results_map) { neigh.push_back(Lpoint(m[0][0], m[0][1], m[0][2])); }
				features(neigh, p);
			}
			tw.stop();
			std::cout << "Time to calculate descriptors: " << tw.getElapsedDecimalSeconds() << " seconds\n";
			debstr += std::to_string(tw.getElapsedDecimalSeconds()) + ", ";

			totPoints.insert(totPoints.end(), points.begin(), points.end());
		}

		string ext = (mainOptions.zip) ? ".laz" : ".las";
		fs::path outputFile = mainOptions.outputDirName / (fileName + "_feat" + std::to_string(rank) + ext);
		tw.start();
		writePointCloudDescriptors(outputFile, totPoints);
		tw.stop();
		std::cout << "Time to write point cloud descriptors: " << tw.getElapsedDecimalSeconds() << " seconds\n";
		debstr += std::to_string(tw.getElapsedDecimalSeconds()) + "\n";

		fs::path debugFile = mainOptions.outputDirName / (fileName + "_deb.csv");
		std::ofstream deb;
		deb.open(debugFile, std::ofstream::app);
		deb << debstr;
		deb.close();

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

	MPI_Finalize();

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