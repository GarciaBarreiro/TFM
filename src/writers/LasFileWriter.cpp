#include "LasFileWriter.hpp"

LASheader _initHeader()
{
    LASheader lasheader;
    lasheader.x_scale_factor = 0.01;
    lasheader.y_scale_factor = 0.01;
    lasheader.z_scale_factor = 0.01;
    lasheader.x_offset = 0.0;   // could maybe be calculated
    lasheader.y_offset = 0.0;
    lasheader.z_offset = 0.0;
    lasheader.point_data_format = 2;    // the one used when reading
    lasheader.point_data_record_length = 26;    // 26 is the minimum record length for 2
    return lasheader;
}

void LasFileWriter::write(std::vector<Lpoint>& points)
{
    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(path.c_str());

    // init header
    LASheader lasheader = _initHeader();

    // init point
    LASpoint laspoint;
    laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

    // open laswriter
    LASwriter* laswriter = laswriteopener.open(&lasheader);
    if (!laswriter)
    {
        std::cout << "ERROR: could not open laswriter\n";
        exit(-2);
    }

    for (const Lpoint& p : points)
    {
        laspoint.set_X(p.getX() * 100);
        laspoint.set_Y(p.getY() * 100);
        laspoint.set_Z(p.getZ() * 100);
        laspoint.set_intensity(p.getI());
        laspoint.set_return_number(p.rn());
        laspoint.set_number_of_returns(p.nor());
        laspoint.set_scan_direction_flag(p.dir());
        laspoint.set_edge_of_flight_line(p.edge());
        laspoint.set_classification(p.getClass());
        laspoint.set_scan_angle_rank(0);    // NO GET
        laspoint.set_user_data(0);          // NO GET
        laspoint.set_point_source_ID(1);    // NO GET
        laspoint.set_R(p.getR());
        laspoint.set_G(p.getG());
        laspoint.set_B(p.getB());

        laswriter->write_point(&laspoint);
        laswriter->update_inventory(&laspoint);
    }

    // update header
    laswriter->update_header(&lasheader, TRUE);

    // outputs bytes written
    laswriter->close();
}

void LasFileWriter::writeDescriptors(std::vector<Lpoint>& points)
{
    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(path.c_str());

    // init header
    LASheader lasheader = _initHeader();

    I32 attribute_index = -1;
    
    I32 type = 4;   // unsigned int
    LASattribute attribute(type, "test", "this is a test attribute");
    attribute.set_scale(1);
    attribute.set_offset(0);
    attribute_index = lasheader.add_attribute(attribute);

    lasheader.update_extra_bytes_vlr();
    lasheader.point_data_record_length += lasheader.get_attributes_size();

    // indices for fast extra bytes access
    I32 attribute_start = lasheader.get_attribute_start(attribute_index);
    
    // init point
    LASpoint laspoint;
    laspoint.init(&lasheader, lasheader.point_data_format, lasheader.point_data_record_length, 0);

    // open laswriter
    LASwriter* laswriter = laswriteopener.open(&lasheader);
    if (!laswriter)
    {
        std::cout << "ERROR: could not open laswriter\n";
        exit(-2);
    }

    unsigned i = 0; // dumb value to use as test attribute
    for (const Lpoint& p : points)
    {
        laspoint.set_X(p.getX() * 100);
        laspoint.set_Y(p.getY() * 100);
        laspoint.set_Z(p.getZ() * 100);
        laspoint.set_intensity(p.getI());
        laspoint.set_return_number(p.rn());
        laspoint.set_number_of_returns(p.nor());
        laspoint.set_scan_direction_flag(p.dir());
        laspoint.set_edge_of_flight_line(p.edge());
        laspoint.set_classification(p.getClass());
        laspoint.set_scan_angle_rank(0);    // NO GET
        laspoint.set_user_data(0);          // NO GET
        laspoint.set_point_source_ID(1);    // NO GET
        laspoint.set_R(p.getR());
        laspoint.set_G(p.getG());
        laspoint.set_B(p.getB());

        laspoint.set_attribute(attribute_start, i++);
        
        laswriter->write_point(&laspoint);
        laswriter->update_inventory(&laspoint);
    }

    // update header
    laswriter->update_header(&lasheader, TRUE);

    // outputs bytes written
    laswriter->close();
}