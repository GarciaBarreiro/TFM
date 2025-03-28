#include "LasFileWriter.hpp"

void LasFileWriter::write(std::vector<Lpoint>& points)
{
    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(path.c_str());

    // init header
    LASheader lasheader;
    lasheader.x_scale_factor = 0.01;
    lasheader.y_scale_factor = 0.01;
    lasheader.z_scale_factor = 0.01;
    lasheader.x_offset = 0.0;   // could maybe be calculated
    lasheader.y_offset = 0.0;
    lasheader.z_offset = 0.0;
    lasheader.point_data_format = 2;    // the one used when reading
    lasheader.point_data_record_length = 26;    // minimum for 2, if we want to add more fields this should increase

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