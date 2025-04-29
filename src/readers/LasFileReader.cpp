//
// Created by miguelyermo on 6/8/21.
//

#include "LasFileReader.hpp"
#include "main_options.hpp"

Lpoint getPoint(unsigned int idx, LASpoint& p, double x, double y, double z)
{
	return Lpoint(idx, x, y, z,
		static_cast<double>(p.get_intensity()),
		static_cast<unsigned short>(p.get_return_number()),
		static_cast<unsigned short>(p.get_number_of_returns()),
		static_cast<unsigned short>(p.get_scan_direction_flag()),
		static_cast<unsigned short>(p.get_edge_of_flight_line()),
		static_cast<unsigned short>(p.get_classification()),
		static_cast<char>(p.get_scan_angle_rank()),
		static_cast<unsigned short>(p.get_user_data()),
		static_cast<unsigned short>(p.get_point_source_ID()),
		static_cast<unsigned int>(p.get_R()),
		static_cast<unsigned int>(p.get_G()),
		static_cast<unsigned int>(p.get_B()));
}

std::vector<Lpoint> LasFileReader::read()
{
	std::vector<Lpoint> points;

	// LAS File reading
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path.c_str());
	LASreader* lasreader = lasreadopener.open();

	// TODO: Use extended_point_records para LAS v1.4
	// https://gitlab.citius.usc.es/oscar.garcia/las-shapefile-classifier/-/blob/master/lasreader_wrapper.cc

	// Scale factors for each coordinate
	double xScale = lasreader->header.x_scale_factor;
	double yScale = lasreader->header.y_scale_factor;
	double zScale = lasreader->header.z_scale_factor;

	double xOffset = lasreader->header.x_offset;
	double yOffset = lasreader->header.y_offset;
	double zOffset = lasreader->header.z_offset;

	// Index of the read point
	unsigned int idx = 0;

	// Main bucle
	while (lasreader->read_point())
	{
		points.emplace_back(getPoint(idx++, lasreader->point, static_cast<double>(lasreader->point.get_X() * xScale + xOffset),
							static_cast<double>(lasreader->point.get_Y() * yScale + yOffset),
		                    static_cast<double>(lasreader->point.get_Z() * zScale + zOffset)));
	}

	delete lasreader;
	return points;
}

std::vector<Lpoint> LasFileReader::readOverlap(const Box& box, const Box& overlap)
{
	std::vector<Lpoint> points;

	// LAS File reading
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path.c_str());
	LASreader* lasreader = lasreadopener.open();

	// Scale factors for each coordinate
	double xScale = lasreader->header.x_scale_factor;
	double yScale = lasreader->header.y_scale_factor;
	double zScale = lasreader->header.z_scale_factor;

	double xOffset = lasreader->header.x_offset;
	double yOffset = lasreader->header.y_offset;
	double zOffset = lasreader->header.z_offset;

	// Index of the read point
	unsigned int idx = 0;

	// Main bucle
	while (lasreader->read_point())
	{
		Point p {static_cast<double>(lasreader->point.get_X() * xScale + xOffset),
				 static_cast<double>(lasreader->point.get_Y() * yScale + yOffset),
				 static_cast<double>(lasreader->point.get_Z() * zScale + zOffset)};
		if (overlap.isInside(p))
		{
			points.emplace_back(getPoint(idx++, lasreader->point, p.getX(), p.getY(), p.getZ()));
			points.back().overlap = !box.isInside(p);
		}
	}

	delete lasreader;
	return points;
}

std::vector<std::vector<Lpoint>> LasFileReader::readOverlap(const std::vector<Box>& boxes, const std::vector<Box>& overlaps)
{
	std::vector<std::vector<Lpoint>> points(boxes.size());

	// LAS File reading
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path.c_str());
	LASreader* lasreader = lasreadopener.open();

	// Scale factors for each coordinate
	double xScale = lasreader->header.x_scale_factor;
	double yScale = lasreader->header.y_scale_factor;
	double zScale = lasreader->header.z_scale_factor;

	double xOffset = lasreader->header.x_offset;
	double yOffset = lasreader->header.y_offset;
	double zOffset = lasreader->header.z_offset;

	// Index of the read point
	unsigned int idx = 0;

	// Main bucle
	while (lasreader->read_point())
	{
		Point p {static_cast<double>(lasreader->point.get_X() * xScale + xOffset),
				 static_cast<double>(lasreader->point.get_Y() * yScale + yOffset),
				 static_cast<double>(lasreader->point.get_Z() * zScale + zOffset)};
		for (int i = 0; i < overlaps.size(); i++)	// points can be saved more than once, not great memory-wise
		{
			if (overlaps[i].isInside(p))
			{
				points[i].emplace_back(getPoint(idx++, lasreader->point, p.getX(), p.getY(), p.getZ()));
				points[i].back().overlap = !boxes[i].isInside(p);
			}
		}
	}

	delete lasreader;
	return points;
}

std::pair<Point, Point> LasFileReader::readBoundingBox()
{
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(path.c_str());
	LASreader* lasreader = lasreadopener.open();

	Point min {lasreader->get_min_x(), lasreader->get_min_y(), lasreader->get_min_z()};
	Point max {lasreader->get_max_x(), lasreader->get_max_y(), lasreader->get_max_z()};

	delete lasreader;
	return std::make_pair(min, max);
}