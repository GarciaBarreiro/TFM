#include "TxtFileWriter.hpp"
#include <fstream>

void TxtFileWriter::write(std::vector<Lpoint>& points)
{
    // TODO: more than 3 columns
    std::ofstream out;
    out.open(path);
    out << std::fixed << std::setprecision(2);

    for (const Lpoint& p : points)
    {
        out << p.getX() << " " << p.getY() << " " << p.getZ() << "\n";
    }

    out.close();
}

void TxtFileWriter::writeDescriptors(std::vector<Lpoint>& points)
{
    std::ofstream out;
    out.open(path);
    out << std::fixed << std::setprecision(2);

    // header with column names
    out << "X Y Z numberNeighbors SumEigenValues Omnivariane Eigenentropy"
            "Linearity Planarity Sphericity CurvatureChange Verticality1"
            "Verticality2 AbsoluteMoment1 AbsoluteMoment2 AbsoluteMoment3"
            "AbsoluteMoment4 AbsoluteMoment5 AbsoluteMoment6 VerticalMoment1"
            "VerticalMoment2";

    for (const Lpoint& p : points)
    {
        out << p.getX() << " " << p.getY() << " " << p.getZ() << " "
        << p.nNeigh << " " << p.omnivar << " " << p.eigenen << " "
        << p.linear << " " << p.planar << " " << p.spheric << " "
        << p.curvChange << " " << p.vert[0] << " " << p.vert[1] << " "
        << p.absMom[0] << " " << p.absMom[1] << " " << p.absMom[2] << " "
        << p.absMom[3] << " " << p.absMom[4] << " " << p.absMom[5] << " "
        << p.vertMom[0] << " " << p.vertMom[1] << "\n";
    }
}