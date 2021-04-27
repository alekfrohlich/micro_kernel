// EPOS RISC-V 32 MMU Mediator Initialization

#include <architecture/mmu.h>
#include <system.h>

__BEGIN_SYS

void MMU::init()
{
    // For machines that do not feature a real MMU, frame size = 1 byte
    // Allocations (using Grouping_List<Frame>::search_decrementing() start from the end
    // To preserve the BOOT stacks until the end of INIT, the free memory list initialization is split in two sections
    // with allocations (from the end) of the first section taking place first
    db<Init, MMU>(TRC) << "MMU::init()" << endl;

    System_Info * si = System::info();
    unsigned sys_data_end = si->lm.sys_data + si->lm.sys_data_size + 1;
    MMU::_master = reinterpret_cast<Page_Directory*>(Memory_Map::PAGE_TABLES);

    db<Init, MMU>(TRC) << "sys_data= " << hex << si->lm.sys_data << ", begin_free= " << hex << sys_data_end << endl; 

    // Worst-Fit guarantees this will work
    free(align_page(sys_data_end), pages(Memory_Map::MMODE_F - align_page(sys_data_end))); // [align_page(&_end), 0x87bf9000]
    free(Memory_Map::MEM_TOP + 1 - Traits<Machine>::STACK_SIZE * Traits<Machine>::CPUS, pages(Traits<Machine>::STACK_SIZE * Traits<Machine>::CPUS));
    
    // Free init/setup memory
    free(Memory_Map::MEM_BASE, pages(Memory_Map::SYS - Memory_Map::MEM_BASE));
}

__END_SYS