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

	// cheesemap
	std::cout << "Building global cheesemap..." << std::endl;
	tw.start();
	const auto flags = chs::flags::build::PARALLEL | chs::flags::build::SHRINK_TO_FIT;
	auto map = chs::Dense<Lpoint, 2>(points, 1.0, flags);
	tw.stop();
	std::cout << "Time to build global cheesemap: " << tw.getElapsedDecimalSeconds() << " seconds\n";

	const auto bytes = map.mem_footprint();
	const auto mb    = bytes / (1024.0 * 1024.0);
	std::cout << "Estimated mem. footprint: " << map.mem_footprint() << " Bytes (" << mb << "MB)" << '\n';

	std::cout << "Number of cells: " << map.get_num_cells() << ", of which, empty: " << map.get_empty_cells() << "\n";

	// neigh search
	const float rad = 2.5;	// search radius
	size_t avg = 0;
	tw.start();
	#pragma omp parallel for reduction(+:avg)
	for (const auto& p : points)
	{
		chs::kernels::Sphere<3> search(p, rad);
		const auto results_map = map.query(search);	// vector with neighs of P inside a sphere of radius rads
		avg += results_map.size();
	}
	tw.stop();
	std::cout << "Average neighbors: " << static_cast<double>(avg) / static_cast<double>(points.size()) <<
				" found in " << tw.getElapsedDecimalSeconds() << " seconds\n";

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

	return EXIT_SUCCESS;
}