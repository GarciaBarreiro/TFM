//
// Created by miguelyermo on 11/3/20.
//

#include "main_options.hpp"

namespace fs = std::filesystem;

main_options mainOptions{};

void printHelp()
{
	std::cout
	    << "-h: Show this message\n"
	       "-i: Path to input file\n"
	       "-o: Path to output file (directory)\n"
		   "-r: Search radius (default: 0)\n"
		   "-R: Enable decimation using (total points)/R points\n"
		   "-s: Cheesemap cell size (default: 1.0)\n"
		   "-z: Write output to LAZ (default: LAS)\n";
	exit(1);
}

void setDefaults()
{
	if (mainOptions.outputDirName.empty()) { mainOptions.outputDirName = "out"; }
}


void processArgs(int argc, char** argv)
{
	while (true)
	{
		const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

		if (-1 == opt) { break; }

		switch (opt)
		{
				// Short Options
			case 'h': {
				printHelp();
				break;
			}
			case 'i': {
				mainOptions.inputFile = fs::path(std::string(optarg));
				std::cout << "Read file set to: " << mainOptions.inputFile << "\n";
				break;
			}
			case 'o': {
				mainOptions.outputDirName = fs::path(std::string(optarg));
				std::cout << "Output path set to: " << mainOptions.outputDirName << "\n";
				break;
			}
			case 'r': {
				mainOptions.radius = std::stof(optarg);
				std::cout << "Search radius set to: " << mainOptions.radius << "\n";
				break;
			}
			case 'R': {
				mainOptions.dec = std::stoi(optarg);
				std::cout << "Decimation enabled and set to: " << mainOptions.dec << "\n";
				break;
			}
			case 's': {
				mainOptions.cellSize = std::stof(optarg);
				std::cout << "Cheesemap cell size set to: " << mainOptions.cellSize << "\n";
				break;
			}
			case 'z': {
				mainOptions.zip = true;
				std::cout << "Set output to LAZ clouds\n";
				break;
			}
			case '?': // Unrecognized option
			default:
				printHelp();
				break;
		}
	}
}