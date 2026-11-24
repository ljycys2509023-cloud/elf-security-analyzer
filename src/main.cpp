#include <iostream>
#include <cstdint>
#include <iomanip>
#include <fstream>
#include "elf_parser.h"
#include "elf_report.h"
using namespace std;


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "usage: elf-analyzer <file>" << '\n';
        return 2;
    }

    ifstream f(argv[1], ios::binary);

    if (!f)
    {
        cerr << "error: cannot open " << argv[1] << '\n';
        return 2;
    }

    unsigned char buf[64]{};
    const unsigned char ELF_MAGIC[4] = {0x7f, 'E', 'L', 'F'};

    f.read(reinterpret_cast<char *>(buf), 64);

    streamsize count = f.gcount();

    if (f.bad())
    {
        cerr << "error: cannot read " << argv[1] << '\n';
        return 2;
    }

    cout << "read " << count << " bytes :";

    for (int i = 0; i < count; i++)
    {
        cout << ' ' << hex << setw(2) << setfill('0')
             << static_cast<int>(buf[i]);

        if ((i + 1) % 16 == 0)
        {
            std::cout << '\n';
        }
    }

    cout << dec << '\n';

    if (count < 64)
    {
        cerr << "ELF header: too short (" << count << " bytes)\n";
        return 1;
    }

    bool match = true;

    for (int i = 0; i < 4; i++)
    {
        if (buf[i] != ELF_MAGIC[i])
        {
            match = false;
            break;
        }
    }

    if (match)
    {
        cout << "ELF magic: match\n";
        Elf64Header hdr{};
        for (int i = 0; i < 16; i++)
        {
            hdr.e_ident[i] = buf[i];
        }
        if (hdr.e_ident[4] == 1)
        {
            cerr << "unsupported: ELF32 is not supported\n";
            return 1;
        }
        if(hdr.e_ident[4] !=1 && hdr.e_ident[4] != 2)
        {
            cerr <<"invalid: unknown ELF class\n";
            return 1;
        }

        if(hdr.e_ident[5] == 2)
        {
            cerr <<"unsupported: big-endian ELF is not supported\n";
            return 1;
        }
        if(hdr.e_ident[5] !=1 && hdr.e_ident[5] !=2)
        {
            cerr <<"invalid: unknown ELF endian\n";
            return 1;
        }

        if(hdr.e_ident[6] != 1)
        {
            cerr << "invalid: unsupported ELF identification version\n";
            return 1;
        }

        parse_elf64_header(buf, hdr);
        
        if(hdr.e_version != 1)
        {
            cerr<< "invalid: unexpected ELF header version\n";
            return 1;
        }
        
        if(hdr.e_ehsize != 64)
        {
            cerr << "invalid: unexpected ELF64 header size\n";
            return 1;
        }
        
        
        print_elf_header(hdr);
        
        return 0;
    }
    else
    {
        cerr << "ELF magic: mismatch\n";
        return 1;
    }
}