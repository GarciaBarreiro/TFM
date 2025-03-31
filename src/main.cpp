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