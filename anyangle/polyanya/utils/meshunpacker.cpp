#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;
typedef unsigned char uchar;
typedef char bint[4];

const uint32_t magic = 0xffffff;

uint32_t remove_b(bint n)
{
    return uint32_t((uchar)(n[0]) << 16 |
                    (uchar)(n[1]) << 8  | (uchar)(n[2]) << 0);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "usage: " << argv[0] << " <file>" << endl;
        return 1;
    }
    string filename = argv[1];
    ifstream packedfile(filename, ios::in | ios::binary);
    if (!packedfile.is_open())
    {
        cerr << "Unable to open file" << endl;
        return 1;
    }

    ofstream meshfile(filename.substr(0, filename.size()-7), ios::out);
    if (!meshfile.is_open())
    {
        cerr << "Unable to open file" << endl;
        return 1;
    }

    bint x;
    if (!packedfile.read(x, 4))
    {
        cerr << "Can't get 4 bytes?" << endl;
        return 1;
    }
    if (strncmp(x, "pack", 4) != 0)
    {
        cerr << "Header is not pack" << endl;
        return 1;
    }
    meshfile << "mesh" << endl;
    uint32_t temp;
    while (packedfile.read(x, 3))
    {
        temp = remove_b(x);
        if (temp == magic)
        {
            meshfile << "-1" << "\n";
        }
        else
        {
            meshfile << temp << "\n";
        }
    }
    return 0;
}
