#include <utility/ostream.h>
#include <utility/elf.h>
#include <architecture.h>
#include <system.h>
#include <time.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Writer beginned" << endl;
    
    CPU::Log_Addr seg_start = shared_seg(port);
    
    
    return 0;
}
