// EPOS RISC-V 32 MMU Mediator Initialization

#include <architecture/mmu.h>

extern "C" void * _edata;
extern "C" void * bss_start;
extern "C" void * _end;

__BEGIN_SYS

void MMU::init()
{
    db<Init, MMU>(TRC) << "MMU::init()" << endl;

    db<Init, MMU>(INF) << "MMU::init::dat.e=" << &_edata << ",bss.b=" << &bss_start << ",bss.e=" << &_end << endl;

    // For machines that do not feature a real MMU, frame size = 1 byte
    // Allocations (using Grouping_List<Frame>::search_decrementing() start from the end
    // To preserve the BOOT stacks until the end of INIT, the free memory list initialization is split in two sections
    // with allocations (from the end) of the first section taking place first

    free(&_end, pages(Traits<Machine>::PAGE_TABLES + 1 - reinterpret_cast<unsigned int>(&_end)));
    free(Memory_Map::MEM_TOP + 1 - Traits<Machine>::STACK_SIZE * Traits<Machine>::CPUS, pages(Traits<Machine>::STACK_SIZE * Traits<Machine>::CPUS));
}

__END_SYS