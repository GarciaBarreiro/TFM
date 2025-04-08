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

void _addAttribute(I32 type, const char *name, LASheader& lasheader, I32& index)
{
    LASattribute attribute(type, name, NULL);
    attribute.set_scale(1);
    attribute.set_offset(0);
    index = lasheader.add_attribute(attribute);
}

void LasFileWriter::writeDescriptors(std::vector<Lpoint>& points)
{
    LASwriteOpener laswriteopener;
    laswriteopener.set_file_name(path.c_str());

    // init header
    LASheader lasheader = _initHeader();
    
    // for specifying type, LASTools seems to use the values found on the LAS spec - 1
    // so if in the spec the value for double is 10, here it is 9
    std::array<I32, 18> indices{};
    _addAttribute(4, "number of neighbors", lasheader, indices[0]); // unsigned int
    _addAttribute(9, "sum of eigenvalues", lasheader, indices[1]);  // double
    _addAttribute(9, "omnivariance", lasheader, indices[2]);
    _addAttribute(9, "eigenentropy", lasheader, indices[3]);
    _addAttribute(9, "linearity", lasheader, indices[4]);
    _addAttribute(9, "planarity", lasheader, indices[5]);
    _addAttribute(9, "sphericity", lasheader, indices[6]);
    _addAttribute(9, "change of curvature", lasheader, indices[7]);
    _addAttribute(9, "verticality 1", lasheader, indices[8]);
    _addAttribute(9, "verticality 2", lasheader, indices[9]);
    _addAttribute(9, "absolute moment 1", lasheader, indices[10]);
    _addAttribute(9, "absolute moment 2", lasheader, indices[11]);
    _addAttribute(9, "absolute moment 3", lasheader, indices[12]);
    _addAttribute(9, "absolute moment 4", lasheader, indices[13]);
    _addAttribute(9, "absolute moment 5", lasheader, indices[14]);
    _addAttribute(9, "absolute moment 6", lasheader, indices[15]);
    _addAttribute(9, "vertical moment 1", lasheader, indices[16]);
    _addAttribute(9, "vertical moment 2", lasheader, indices[17]);

    lasheader.update_extra_bytes_vlr();
    lasheader.point_data_record_length += lasheader.get_attributes_size();

    // indices for fast extra bytes access
    std::array<I32, 18> att_starts{};
    for (int i = 0; i < indices.size(); i++)
    {
        att_starts[i] = lasheader.get_attribute_start(indices[i]);
    }
    
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

        laspoint.set_attribute(att_starts[0], p.nNeigh);
        laspoint.set_attribute(att_starts[1], p.sum);
        laspoint.set_attribute(att_starts[2], p.omnivar);
        laspoint.set_attribute(att_starts[3], p.eigenen);
        laspoint.set_attribute(att_starts[4], p.linear);
        laspoint.set_attribute(att_starts[5], p.planar);
        laspoint.set_attribute(att_starts[6], p.spheric);
        laspoint.set_attribute(att_starts[7], p.curvChange);
        laspoint.set_attribute(att_starts[8], p.vert[0]);
        laspoint.set_attribute(att_starts[9], p.vert[1]);
        laspoint.set_attribute(att_starts[10], p.absMom[0]);
        laspoint.set_attribute(att_starts[11], p.absMom[1]);
        laspoint.set_attribute(att_starts[12], p.absMom[2]);
        laspoint.set_attribute(att_starts[13], p.absMom[3]);
        laspoint.set_attribute(att_starts[14], p.absMom[4]);
        laspoint.set_attribute(att_starts[15], p.absMom[5]);
        laspoint.set_attribute(att_starts[16], p.vertMom[0]);
        laspoint.set_attribute(att_starts[17], p.vertMom[1]);
        
        laswriter->write_point(&laspoint);
        laswriter->update_inventory(&laspoint);
    }

    // update header
    laswriter->update_header(&lasheader, TRUE);

    // outputs bytes written
    laswriter->close();
}