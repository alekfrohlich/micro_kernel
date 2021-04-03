#include <utility/ostream.h>
#include <process.h>
#include <memory.h>
#include <architecture/rv32/rv32_mmu.h>

using namespace EPOS;

OStream cout;

typedef unsigned int RV_INS;

int main()
{

    cout << "Hello world" << endl;
    Segment * code_seg = new Segment(1024*4, MMU::RV32_Flags::SYS);
    Segment * data_seg = new Segment(1024*4, MMU::RV32_Flags::SYS);
    cout << "Segments ready" << endl;
    Task * t =  new Task(code_seg, data_seg);

    cout << "End!" << endl;
    return 0;
}
