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
        cout << "e_type    = " << read_u16(buf + 16) << '\n';
        cout << "e_machine = " << read_u16(buf + 18) << '\n';
        cout << "e_version = " << read_u32(buf + 20) << '\n';
        cout << "e_entry   = 0x" << hex << read_u64(buf + 24) << dec << '\n';
        return 0;
    }
    else
    {
        cout << "ELF magic: mismatch\n";
        return 1;
    }
}