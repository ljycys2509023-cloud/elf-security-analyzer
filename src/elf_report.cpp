#include "elf_report.h"
#include <iostream>
#include <iomanip>
using namespace std;

void print_elf_header(const Elf64Header &hdr)
{
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
}