// EPOS PC Scratchpad Memory Initialization

#include <machine/riscv/riscv_scratchpad.h>
#include <system.h>
#include <memory.h>

__BEGIN_SYS

void Scratchpad::init()
{
    db<Init, Scratchpad>(TRC) << "Scratchpad::init(a=" << ADDRESS << ",s=" << SIZE << ")" << endl;

    _segment = new (SYSTEM) Segment(CPU::Phy_Addr(ADDRESS), SIZE, Segment::Flags::SYS);
    _heap = new (SYSTEM) Heap(Address_Space(MMU::current()).attach(_segment), _segment->size());
}

__END_SYS