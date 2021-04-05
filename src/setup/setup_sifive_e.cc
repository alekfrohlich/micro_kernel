// EPOS RISC-V sifive SETUP

#include <architecture.h>
#include <machine.h>

using namespace EPOS::S;
typedef unsigned int Reg;

extern "C"
{
    [[gnu::naked, gnu::section(".init")]] void _setup();
    void _int_entry();
    void _start();
    void _wait() {
        CPU::halt();
        _start();
    }
}
bool passei = false;
extern "C" [[gnu::interrupt, gnu::aligned(4)]] void _mmode_forward() {
    Reg id = CPU::mcause();
    passei = true;
    if((id & IC::INT_MASK) == CLINT::IRQ_MAC_TIMER) {
        Timer::reset();
        CPU::sie(CPU::STI);
    }
    Reg interrupt_id = 1 << ((id & IC::INT_MASK) - 2);
    if(CPU::int_enabled() && (CPU::sie() & (interrupt_id)))
        CPU::mip(interrupt_id);
}

__BEGIN_SYS

class Setup_SifiveE {
private:
    typedef CPU::Reg Reg;
    typedef MMU::RV32_Flags RV32_Flags;
    typedef MMU::Page_Table Page_Table;
    typedef MMU::Page_Directory Page_Directory;
    typedef MMU::PT_Entry PT_Entry;

public:
    static void init() { setup_machine_environment(); }
    static void setup_machine_environment();
    static void setup_supervisor_environment();
    static void build_page_tables();
    static void clean_bss();
};

void Setup_SifiveE::build_page_tables()
{
    // Address of the Directory
    Reg page_tables = Traits<Machine>::PAGE_TABLES;
    MMU::_master = new ( (void *) page_tables ) Page_Directory();

    // Number of kernel entries in each directory
    unsigned sys_entries = 512 + MMU::page_tables(MMU::pages(Traits<Machine>::MEM_TOP + 1 - Traits<Machine>::MEM_BASE));

    MMU::_master->remap(page_tables + 4096, RV32_Flags::VALID, 0, sys_entries);

    // Map logical addrs back to themselves; with this, the kernel may access any
    // physical RAM address directly (as if paging wasn't there)
    for(unsigned i = 0; i < sys_entries; i++)
    {
        Page_Table * pt = new ( (void *)(page_tables + 4*1024*(i+1)) ) Page_Table();
        pt->remap(i * 1024*4096, RV32_Flags::SYS);
    }
}

extern "C" char __bss_start;
extern "C" char _end;

void Setup_SifiveE::clean_bss()
{
    unsigned * bss_start = reinterpret_cast<unsigned *>(&__bss_start);
    unsigned * bss_end = reinterpret_cast<unsigned *>(&_end);
    for (unsigned * word = bss_start; word < bss_end; word++) {
        unsigned * t = new (word) unsigned;
        *t = 0;
    }
}

void Setup_SifiveE::setup_supervisor_environment()
{
    CPU::stvec_write((unsigned)&_int_entry & 0xfffffffc);

    // We must clean the bss before setting MMU::_master
    clean_bss();

    // This creates and configures the kernel page tables (which map logical==physical)
    build_page_tables();

    // forward everything
    CPU::satp((0x1 << 31) | (Traits<Machine>::PAGE_TABLES >> 12));
    CPU::sepc_write((unsigned)&_start);

    // Interrupts will remain disable until the Context::load at Init_First
    CPU::sstatus_write(CPU::SPP_S);
    CPU::sie_write(CPU::SSI | CPU::STI | CPU::SEI);

    ASM("sret");
}

void Setup_SifiveE::setup_machine_environment()
{
    // We first configure the M-mode CSRs and then switch to S-mode
    // configure paging. After that, we won't return to M-mode; an exception
    // is the forwarding of ints and excps to S-mode.

    CPU::mie_write(CPU::MSI | CPU::MTI | CPU::MEI);
    CPU::mmode_int_disable();

    // We need to set:
    //      MPP_S: to switch to S-mode after mret
    //      MPIE:  otherwise we won't ever receive interrupts
    CPU::mstatus_write(CPU::MPP_S | CPU::MPIE);

    // We store mhartid at tp, since it becomes inaccessible while in S-mode.
    Reg core = CPU::mhartid();
    CPU::tp(core);

    // Set stack for each core
    CPU::sp(Traits<Machine>::BOOT_STACK - Traits<Machine>::STACK_SIZE * core);

    // Guarantee that paging is off before going to S-mode.
    CPU::satp(0);

    // Forward all ints and excs to S-mode.
    //!ECALLS: Not yet implemented.
    CPU::mideleg_write(CPU::SSI | CPU::STI | CPU::SEI);
    CPU::medeleg_write(0xffff);

    // All ints received in M-mode are forwarded to S-mode.
    // The first two bits indicate the mode: Direct or Vectored;
    // we opted for Direct.
    CPU::mtvec((unsigned)&_mmode_forward & 0xfffffffc);
    CPU::mepc((unsigned)&setup_supervisor_environment);

    ASM("mret");
}

__END_SYS

void _setup() { Setup_SifiveE::init(); }
