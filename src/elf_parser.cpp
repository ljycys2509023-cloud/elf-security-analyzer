#include "elf_parser.h"
#include <cstdint>

uint16_t read_u16(const unsigned char *p)
{
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t read_u32(const unsigned char *p)
{
    return static_cast<uint32_t>(p[0])
         | (static_cast<uint32_t>(p[1]) << 8)
         | (static_cast<uint32_t>(p[2]) << 16)
         | (static_cast<uint32_t>(p[3]) << 24);
}

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

void parse_elf64_header(const unsigned char *buf, Elf64Header &hdr)
{
    hdr.e_type = read_u16(buf + 16);
    hdr.e_machine = read_u16(buf + 18);
    hdr.e_version = read_u32(buf + 20);
    hdr.e_entry = read_u64(buf + 24);
    hdr.e_phoff = read_u64(buf + 32);
    hdr.e_shoff = read_u64(buf + 40);
    hdr.e_flags = read_u32(buf + 48);
    hdr.e_ehsize = read_u16(buf + 52);
    hdr.e_phentsize = read_u16(buf + 54);
    hdr.e_phnum = read_u16(buf + 56);
    hdr.e_shentsize = read_u16(buf + 58);
    hdr.e_shnum = read_u16(buf + 60);
    hdr.e_shstrndx = read_u16(buf + 62);

}

