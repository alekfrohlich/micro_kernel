#include <utility/ostream.h>
#include <process.h>
#include <memory.h>
#include <architecture/rv32/rv32_mmu.h>

using namespace EPOS;

OStream cout;

int main()
{
    cout << "Task Test" << endl;
    unsigned * data_base = reinterpret_cast<unsigned*>(0xFFBFFFFF);
    
    Segment * code_seg1 = new Segment(1024*4, MMU::Flags::KCODE);
    Segment * data_seg1 = new Segment(1024*4, MMU::Flags::KDATA);
    cout << "Create Task1" << endl;
    Task * t1 =  new Task(code_seg1, data_seg1);

    ASM("A1:");
    t1->activate();
    *data_base = 10;

    Segment * code_seg2 = new Segment(1024*4, MMU::Flags::KCODE);
    Segment * data_seg2 = new Segment(1024*4, MMU::Flags::KDATA);
    cout << "Create Task2" << endl;
    Task * t2 =  new Task(code_seg2, data_seg2);

    // Should be 10
    cout << "Data base=" << *data_base << endl;
    ASM("A2:");
    t2->activate();
    *data_base = 9;
    // Should be 9
    cout << "Data base=" << *data_base << endl;

    ASM("End:");
    cout << "End!" << endl;
    return 0;
}
