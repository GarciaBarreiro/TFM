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
        if (p.overlap) continue;
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

I32 _addAttribute(LASheader& lasheader, I32 type, F64 scale, const char *name)
{
    LASattribute attribute(type, name, NULL);
    attribute.set_scale(scale);
    attribute.set_offset(0);
    return lasheader.add_attribute(attribute);
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
    indices[0]  = _addAttribute(lasheader, 4,     1, "number of neighbors");    // unsigned int
    indices[1]  = _addAttribute(lasheader, 4, 0.001, "sum of eigenvalues");
    indices[2]  = _addAttribute(lasheader, 2, 0.001, "omnivariance");           // unsigned short
    indices[3]  = _addAttribute(lasheader, 5, 0.001, "eigenentropy");           // int
    indices[4]  = _addAttribute(lasheader, 3, 0.001, "linearity");              // short
    indices[5]  = _addAttribute(lasheader, 3, 0.001, "planarity");
    indices[6]  = _addAttribute(lasheader, 2, 0.001, "sphericity");             // unsigned short
    indices[7]  = _addAttribute(lasheader, 2, 0.001, "change of curvature");
    indices[8]  = _addAttribute(lasheader, 2, 0.001, "verticality [0]");
    indices[9]  = _addAttribute(lasheader, 2, 0.001, "verticality [1]");
    indices[10] = _addAttribute(lasheader, 2, 0.001, "absolute moment [0]");
    indices[11] = _addAttribute(lasheader, 4, 0.001, "absolute moment [1]");    // unsigned int
    indices[12] = _addAttribute(lasheader, 2, 0.001, "absolute moment [2]");    // unsigned short
    indices[13] = _addAttribute(lasheader, 4, 0.001, "absolute moment [3]");    // unsigned int
    indices[14] = _addAttribute(lasheader, 2, 0.001, "absolute moment [4]");    // unsigned short
    indices[15] = _addAttribute(lasheader, 4, 0.001, "absolute moment [5]");    // unsigned int
    indices[16] = _addAttribute(lasheader, 5, 0.001, "vertical moment [0]");    // int
    indices[17] = _addAttribute(lasheader, 5, 0.001, "vertical moment [1]");

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
        if (p.overlap) continue;
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
        // check if NaN, else casting results because if not, writes noise to all attributes
        laspoint.set_attribute(att_starts[1], std::isnan(p.sum) ? 0 : U32_QUANTIZE(1000 * p.sum));
        laspoint.set_attribute(att_starts[2], std::isnan(p.omnivar) ? 0 : U16_QUANTIZE(1000 * p.omnivar));
        laspoint.set_attribute(att_starts[3], std::isnan(p.eigenen) ? 0 : I32_QUANTIZE(1000 * p.eigenen));
        laspoint.set_attribute(att_starts[4], std::isnan(p.linear) ? 0 : I16_QUANTIZE(1000 * p.linear));
        laspoint.set_attribute(att_starts[5], std::isnan(p.planar) ? 0 : I16_QUANTIZE(1000 * p.planar));
        laspoint.set_attribute(att_starts[6], std::isnan(p.spheric) ? 0 : U16_QUANTIZE(1000 * p.spheric));
        laspoint.set_attribute(att_starts[7], std::isnan(p.curvChange) ? 0 : U16_QUANTIZE(1000 * p.curvChange));
        laspoint.set_attribute(att_starts[8], std::isnan(p.vert[0]) ? 0 : U16_QUANTIZE(1000 * p.vert[0]));
        laspoint.set_attribute(att_starts[9], std::isnan(p.vert[1]) ? 0 : U16_QUANTIZE(1000 * p.vert[1]));
        laspoint.set_attribute(att_starts[10], std::isnan(p.absMom[0]) ? 0 : U16_QUANTIZE(1000 * p.absMom[0]));
        laspoint.set_attribute(att_starts[11], std::isnan(p.absMom[1]) ? 0 : U32_QUANTIZE(1000 * p.absMom[1]));
        laspoint.set_attribute(att_starts[12], std::isnan(p.absMom[2]) ? 0 : U16_QUANTIZE(1000 * p.absMom[2]));
        laspoint.set_attribute(att_starts[13], std::isnan(p.absMom[3]) ? 0 : U32_QUANTIZE(1000 * p.absMom[3]));
        laspoint.set_attribute(att_starts[14], std::isnan(p.absMom[4]) ? 0 : U16_QUANTIZE(1000 * p.absMom[4]));
        laspoint.set_attribute(att_starts[15], std::isnan(p.absMom[5]) ? 0 : U32_QUANTIZE(1000 * p.absMom[5]));
        laspoint.set_attribute(att_starts[16], std::isnan(p.vertMom[0]) ? 0 : I32_QUANTIZE(1000 * p.vertMom[0]));
        laspoint.set_attribute(att_starts[17], std::isnan(p.vertMom[1]) ? 0 : I32_QUANTIZE(1000 * p.vertMom[1]));
        
        laswriter->write_point(&laspoint);
        laswriter->update_inventory(&laspoint);
    }

    // update header
    laswriter->update_header(&lasheader, TRUE);

    // outputs bytes written
    laswriter->close();
}