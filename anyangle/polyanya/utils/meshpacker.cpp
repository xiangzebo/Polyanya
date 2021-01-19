#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void print_int(uint32_t n, ostream& s)
{
    const char z = static_cast<char>(0xFF);
    const char bytes[] = {static_cast<char>((n>>16)&z), static_cast<char>((n>>8)&z), static_cast<char>((n>>0)&z)};
    s.write(bytes, 3);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "usage: " << argv[0] << " <file>" << endl;
        return 1;
    }
    string filename = argv[1];
    ifstream meshfile(filename);
    if (!meshfile.is_open())
    {
        cerr << "Unable to open file" << endl;
        return 1;
    }
    ofstream packedfile(filename + ".packed", ios::out | ios::binary);
    if (!packedfile.is_open())
    {
        cerr << "Unable to open file" << endl;
        return 1;
    }
    string header;
    if (!(meshfile >> header))
    {
        cerr << "No header!" << endl;
        return 1;
    }
    if (header != "mesh")
    {
        cerr << "Header is not mesh!" << endl;
        return 1;
    }
    int temp;
    packedfile.write("pack", 4);
    while (meshfile >> temp)
    {
        print_int(temp, packedfile);
    }
    return 0;
}
