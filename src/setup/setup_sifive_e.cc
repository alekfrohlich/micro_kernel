// EPOS RISC-V sifive SETUP

#include <system/config.h>
#include <architecture/cpu.h>
#include <architecture/mmu.h>
#include <machine/timer.h>
#include <machine/ic.h>

using namespace EPOS::S;

extern "C" 
{
    void _setup() __attribute__ ((used, naked, section(".init")));
    void _int_entry();
    void _mmode_forward();
    void _start();
    void _wait() 
    { 
        // is it possible that wfi is ignored?
        CPU::halt();
        _start();
    }
}

__BEGIN_SYS

class Setup_SifiveE {
private:
    typedef CPU::Reg Reg;
    // typedef CPU::Phy_Addr Phy_Addr;
    // typedef CPU::Log_Addr Log_Addr;
    // typedef MMU::Page Page;
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

//!SMODE: we are mapping MMIO
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
    CPU::sstatus_write( CPU::SIE | CPU::SPIE | CPU::SPP_S );
    CPU::sie_write( CPU::SSI | CPU::STI | CPU::SEI );
    CPU::stvec_write((unsigned)&_int_entry & 0xfffffffe);
    CPU::sepc_write((unsigned)&_start);
    CPU::satp((0x1 << 31) | (Traits<Machine>::PAGE_TABLES >> 12));
    ASM("sret");
}

//!SMODE: tp is being used in context switching
void Setup_SifiveE::setup_machine_environment()
{
    CPU::mmode_int_disable();
    Reg core = CPU::mhartid();
    CPU::tp(core);
    // set stack for each core
    CPU::sp(Traits<Machine>::BOOT_STACK - Traits<Machine>::STACK_SIZE * core);
    CPU::mstatus_write(CPU::MPP_S | CPU::MPIE);
    CPU::mepc((unsigned)&setup_supervisor_environment);
    CPU::mtvec((unsigned)&_mmode_forward & 0xfffffffe);

    if (core == 0) {
        build_page_tables();
        CPU::satp_write(0); // paging off
        
        //!SMODE: How is mie being activated?
        // CPU::mie_write(CPU::MTI);
        
        //!SMODE: does each core has its mi(e)deleg?
        // forward everything
        CPU::mideleg_write(CPU::SSI | CPU::STI | CPU::SEI);
        CPU::medeleg_write(0xffff);
        ASM("mret");
    } else { // Not tested
        // CPU::mstatus_write(CPU::MSTATUS_DEFAULTS);
        // CPU::mie(CPU::MSI | CPU::MTI | CPU::MEI);
        // CPU::mepc((unsigned)&_wait);
        // ASM("mret");
    }
}

__END_SYS

void _setup() { Setup_SifiveE::init(); }
