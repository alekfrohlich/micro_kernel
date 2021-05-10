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

    free(align_page(sys_data_end), pages(Memory_Map::MMODE_F - align_page(sys_data_end)));
    // This region is used by _mmode_forward as a M-mode stack
    // free(Memory_Map::MEM_TOP + 1 - Traits<Machine>::STACK_SIZE * Traits<Machine>::CPUS, pages(Traits<Machine>::STACK_SIZE * Traits<Machine>::CPUS));
    
    // Free init/setup memory
    free(Memory_Map::MEM_BASE, pages(Memory_Map::SYS - Memory_Map::MEM_BASE));
    db<Init, MMU>(TRC) << "MMU has been given the regions: [" << hex << align_page(sys_data_end) << "," << hex << Memory_Map::MMODE_F << "], [" \
        << hex << Memory_Map::MEM_BASE << "," << hex << Memory_Map::SYS << "]" << endl;
}

__END_SYS