#include "TxtFileWriter.hpp"
#include <fstream>

void TxtFileWriter::write(std::vector<Lpoint>& points)
{
    // TODO: more than 3 columns
    std::ofstream out;
    out.open(path);
    out << std::fixed << std::setprecision(5);

    for (const Lpoint& p : points)
    {
        out << p.getX() << " " << p.getY() << " " << p.getZ() << "\n";
    }

    out.close();
}