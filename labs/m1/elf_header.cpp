#include <iostream>
#include <cstdint>
#include <iomanip>
#include <fstream>
using namespace std;

struct Elf64Header
{
    uint8_t e_ident[16];  // Magic number and other info
    uint16_t e_type;      // Object file type
    uint16_t e_machine;   // Architecture
    uint32_t e_version;   // Object file version
    uint64_t e_entry;     // Entry point virtual address
    uint64_t e_phoff;     // Program header table file offset
    uint64_t e_shoff;     // Section header table file offset
    uint32_t e_flags;     // Processor-specific flags
    uint16_t e_ehsize;    // ELF header size in bytes
    uint16_t e_phentsize; // Program header table entry size
    uint16_t e_phnum;     // Program header table entry count
    uint16_t e_shentsize; // Section header table entry size
    uint16_t e_shnum;     // Section header table entry count
    uint16_t e_shstrndx;  // Section header string table index
};

// Read a 2-byte little-endian value starting at p
uint16_t read_u16(const unsigned char *p)
{
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

// Read a 4-byte little-endian value starting at p
uint32_t read_u32(const unsigned char *p)
{
    return static_cast<uint32_t>(p[0])
         | (static_cast<uint32_t>(p[1]) << 8)
         | (static_cast<uint32_t>(p[2]) << 16)
         | (static_cast<uint32_t>(p[3]) << 24);
}

// Read an 8-byte little-endian value starting at p
uint64_t read_u64(const unsigned char *p)
{
    return static_cast<uint64_t>(p[0])
         | (static_cast<uint64_t>(p[1]) << 8)
         | (static_cast<uint64_t>(p[2]) << 16)
         | (static_cast<uint64_t>(p[3]) << 24)
         | (static_cast<uint64_t>(p[4]) << 32)
         | (static_cast<uint64_t>(p[5]) << 40)
         | (static_cast<uint64_t>(p[6]) << 48)
         | (static_cast<uint64_t>(p[7]) << 56);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "usage: elf_header <file>" << '\n';
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
        cout << "ELF header: too short (" << count << " bytes)\n";
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

        hdr.e_type = read_u16(buf + 16);
        hdr.e_machine = read_u16(buf + 18);
        hdr.e_version = read_u32(buf + 20);
        if(hdr.e_version != 1)
        {
            cerr<< " invalid: unexpected ELF header version\n";
            return 1;
        }
        hdr.e_entry = read_u64(buf + 24);
        hdr.e_phoff = read_u64(buf + 32);
        hdr.e_shoff = read_u64(buf + 40);
        hdr.e_flags = read_u32(buf + 48);
        hdr.e_ehsize = read_u16(buf + 52);
        if(hdr.e_ehsize != 64)
        {
            cerr << "invalid: unexpected ELF64 header size\n";
            return 1;
        }
        hdr.e_phentsize = read_u16(buf + 54);
        hdr.e_phnum = read_u16(buf + 56);
        hdr.e_shentsize = read_u16(buf + 58);
        hdr.e_shnum = read_u16(buf + 60);
        hdr.e_shstrndx = read_u16(buf + 62);
        
        cout << "\nELF Header:\n";
        cout << "Class: ELF64\n";
        cout << "Data: Little endian\n";
        cout << "e_type      = " << hdr.e_type << '\n';
        if(hdr.e_type == 3)
        {
            cout << "Type: DYN (3)\n";
        }
        else if(hdr.e_type == 2)
        {
            cout << "Type: EXEC (2)\n";
        }
        else if(hdr.e_type == 1)
        {
            cout << "Type: REL (1)\n";
        }
        else
        {
            cout << "Type: OTHER\n";
        }
        cout << "e_machine   = " << hdr.e_machine << '\n';
        if(hdr.e_machine == 62)
        {
            cout << "Machine: AMD x86-64 (62)\n";
        }
        else if(hdr.e_machine == 183)
        {
            cout << "Machine: AArch64 (183)\n";
        }
        else
        {
            cout<< "Machine: OTHER\n";
        }
        cout << "e_version   = " << hdr.e_version << '\n';
        cout << "e_entry     = 0x" << hex << hdr.e_entry << dec << '\n';
        cout << "e_phoff     = "<< hdr.e_phoff <<'\n';
        cout << "e_shoff     = "<< hdr.e_shoff <<'\n';
        cout << "e_flags     = "<< hdr.e_flags <<'\n';
        cout << "e_ehsize    = "<< hdr.e_ehsize <<'\n';
        cout << "e_phentsize = "<< hdr.e_phentsize <<'\n';
        cout << "e_phnum     = "<< hdr.e_phnum <<'\n';
        cout << "e_shentsize = "<< hdr.e_shentsize <<'\n';
        cout << "e_shnum     = "<< hdr.e_shnum <<'\n';
        cout << "e_shstrndx  = "<< hdr.e_shstrndx <<'\n';
        
        return 0;
    }
    else
    {
        cout << "ELF magic: mismatch\n";
        return 1;
    }
}