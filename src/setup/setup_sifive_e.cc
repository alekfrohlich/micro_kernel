// EPOS RISC-V sifive SETUP

#include <system/config.h>
#include <architecture/cpu.h>
#include <architecture/mmu.h>
#include <machine/timer.h>
#include <machine/ic.h>

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

extern "C" [[gnu::interrupt, gnu::aligned(4)]] void _mmode_forward() {
    Reg id = CPU::mcause();
    if(id & CLINT::INTERRUPT) {
        if((id & IC::INT_MASK) == CLINT::IRQ_MAC_TIMER) {
            Timer::reset();
            CPU::sie(CPU::STI);
        }
        Reg interrupt_id = 1 << ((id & IC::INT_MASK) - 2);
        if(CPU::int_enabled() && (CPU::sie() & (interrupt_id)))
            CPU::mip(interrupt_id);
    }
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
};

void Setup_SifiveE::build_page_tables() 
{
    Reg page_tables = Traits<Machine>::PAGE_TABLES; // address of the page table root
    MMU::_master = new ( (void *) page_tables ) Page_Directory();

    for(int i = 0; i < 1024; i++) { 
        PT_Entry * pte = (((PT_Entry *)MMU::_master) + i);
        * pte = ((page_tables >> 12) << 10);
        * pte += ((i+1) << 10);
        * pte |= MMU::RV32_Flags::VALID;    
    }

    for(int i = 0; i < 1024; i++)
    {
        Page_Table * pt = new ( (void *)(page_tables + 4*1024*(i+1))  ) Page_Table();
        pt->remap(RV32_Flags::SYS);
    }
}

void Setup_SifiveE::setup_supervisor_environment() 
{
    IC::init();
    IC::int_vector(IC::INT_RESCHEDULER, IC::ipi_eoi);
    CPU::stvec_write((unsigned)&_int_entry & 0xfffffffc);
    build_page_tables();
    
    // forward everything
    CPU::satp((0x1 << 31) | (Traits<Machine>::PAGE_TABLES >> 12));
    CPU::sepc_write((unsigned)&_start);
    CPU::sstatus_write(CPU::SPP_S);
    CPU::sie_write(CPU::SSI | CPU::STI | CPU::SEI);
    ASM("sret");
}

void Setup_SifiveE::setup_machine_environment()
{
    CPU::mie_write(CPU::MSI | CPU::MTI | CPU::MEI);
    CPU::mmode_int_disable();

    CPU::mstatus_write(CPU::MPP_S | CPU::MPIE);
    Reg core = CPU::mhartid();
    CPU::tp(core);
    // set stack for each core
    CPU::sp(Traits<Machine>::BOOT_STACK - Traits<Machine>::STACK_SIZE * core);
    CPU::satp_write(0); // paging off
    CPU::mideleg_write(CPU::SSI | CPU::STI | CPU::SEI);
    CPU::medeleg_write(0xffff); // ecalls will be handled in the future
    CPU::mtvec((unsigned)&_mmode_forward & 0xfffffffc);
    CPU::mepc((unsigned)&setup_supervisor_environment);

    ASM("mret");
}

__END_SYS

void _setup() { Setup_SifiveE::init(); }
