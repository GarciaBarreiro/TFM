//
// Created by miguelyermo on 1/3/20.
//

#include "handlers.hpp"
#include "FileReaderFactory.hpp"
#include "FileWriterFactory.hpp"

void createDirectory(const fs::path& dirName)
/**
 * This function creates a directory if it does not exist.
 * @param dirname
 * @return
 */
{
	if (!fs::is_directory(dirName)) { fs::create_directories(dirName); }
}

std::vector<Lpoint> readPointCloud(const fs::path& filename)
{
	// Get Input File extension
	auto fExt = filename.extension();

	File_t readerType = chooseReaderType(fExt);

	// asdf
	if (readerType == err_t)
	{
		std::cout << "Uncompatible file format\n";
		exit(-1);
	}

	std::shared_ptr<FileReader> fileReader = FileReaderFactory::makeReader(readerType, filename);

	std::vector<Lpoint> points = fileReader->read();
	// Decimation. Implemented here because, tbh, I don't want to implement it for each reader type.
	std::cout << "Point cloud size: " << points.size() << "\n";

	return points;
}

void writePointCloud(const fs::path& fileName, std::vector<Lpoint>& points)
{
	// get output file extension
	auto fExt = fileName.extension();

	File_t writerType = chooseWriterType(fExt);

	if (writerType == err_t)
	{
		std::cout << "Uncompatible file format\n";
		exit(-1);
	}

	std::shared_ptr<FileWriter> fileWriter = FileWriterFactory::makeWriter(writerType, fileName);

	fileWriter->write(points);
}

void writePointCloudDescriptors(const fs::path& fileName, std::vector<Lpoint>& points)
{
	// get output file extension
	auto fExt = fileName.extension();

	File_t writerType = chooseWriterType(fExt);

	if (writerType == err_t)
	{
		std::cout << "Uncompatible file format\n";
		exit(-1);
	}

	std::shared_ptr<FileWriter> fileWriter = FileWriterFactory::makeWriter(writerType, fileName);

	fileWriter->writeDescriptors(points);
}