#include <utility/ostream.h>
#include <utility/elf.h>
#include <architecture.h>
#include <system.h>
#include <time.h>
#include <memory.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Writer beginned" << endl;
    
    // unsigned int task;
    unsigned int port = 1;
    Shared_Segment * sseg = new Shared_Segment(port, 1024, MMU::Flags::UDATA);
    cout << sseg << endl;
    // sseg->add
    // CPU::Log_Addr seg_start = shared_seg(port, frames);
    
    
    return 0;
}
