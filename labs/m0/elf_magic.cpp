#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "usage: elf_magic <file>" << '\n';
        return 2;
    }

    ifstream f(argv[1], ios::binary);

    if (!f)
    {
        cerr << "error: cannot open " << argv[1] << '\n';
        return 2;
    }

    unsigned char buf[4]{};
    const unsigned char ELF_MAGIC[4] = { 0x7F, 'E', 'L', 'F' };

    f.read(reinterpret_cast<char*>(buf), 4);

    streamsize count = f.gcount();

    cout << "read " << count << " bytes:";

    for (int i = 0; i < count; i++)
    {
        cout << ' ' << hex << setw(2) << setfill('0')
            << static_cast<int>(buf[i]);
    }

    cout << dec << '\n';

    // 先检查是否读满了 4 个字节
    if (count < 4)
    {
        cout << "ELF magic: too short (" << count << " bytes)\n";
        return 1;
    }

    // 逐个比较魔数
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
        return 0;
    }
    else
    {
        cout << "ELF magic: mismatch\n";
        return 1;
    }
}