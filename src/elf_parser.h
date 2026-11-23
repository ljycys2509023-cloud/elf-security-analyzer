#pragma once
#include <cstdint>

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

uint16_t read_u16(const unsigned char *p);

uint32_t read_u32(const unsigned char *p);

uint64_t read_u64(const unsigned char *p);

void parse_elf64_header(const unsigned char *buf, Elf64Header &hdr);