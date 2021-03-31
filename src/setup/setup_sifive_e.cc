// EPOS RISC-V sifive SETUP

//!SMODE: this imports could be made softier
#include <system/config.h>
#include <architecture/cpu.h>
#include <architecture/mmu.h>
#include <machine/timer.h>
#include <machine/ic.h>

using namespace EPOS::S;
typedef unsigned int Reg;

extern "C" 
{
    void _setup() __attribute__ ((used, naked, section(".init")));
    void _mmode_forward();
    void int_m2s() __attribute__ ((naked));
    void _int_entry();
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

    // for(int i = 0; i < 1024; i++)
    // {
    //     Page_Table * pt = new ( (void *)(page_tables + 4*1024*(i+1))  ) Page_Table();
    //     pt->remap(RV32_Flags::SYS);
    // }
    for(int i = 0; i < 512; i++)
    {
        Page_Table * pt = new ( (void *)(page_tables + 4*1024*(i+1))  ) Page_Table();
        pt->remap(RV32_Flags::SYS);
    }

    ASM("bk:");
    Page_Table * pt = new ( (void *)(page_tables + 4*1024*(512+1))  ) Page_Table();
    pt->ptes[0] = RV32_Flags::SYS;
    for(int i = 1; i < 1024; i++) {
        unsigned int pte = (((unsigned)pt - Traits<Machine>::PAGE_TABLES)>>12) - 1;
        pte = pte << 20;
        pte += ((i) << 10);
        pte = pte | RV32_Flags::SYS;
        pt->ptes[i] = pte;
    }

    for(int i = 513; i < 1024; i++)
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
    // CPU::sstatus_write(CPU::SIE | CPU::SPIE | CPU::SPP_S);
    CPU::sstatus_write(CPU::SPP_S);
    CPU::sie_write(CPU::SSI | CPU::STI | CPU::SEI);
    ASM("sret");
}

//!SMODE: tp is being used in context switching
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
    CPU::medeleg_write(0xffff);
    CPU::mtvec((unsigned)&int_m2s & 0xfffffffc);
    CPU::mepc((unsigned)&setup_supervisor_environment);

    ASM("mret");
}

extern "C" void int_m2s() {
    // saving and restoring registers since we are making more than just executing mret
    ASM("        .align 4                                               \n"
        "                                                               \n"
        "# Save context                                                 \n"
        "        addi        sp,     sp,   -124                         \n"          // 32 regs of 4 bytes each = 128 Bytes
        "        sw          x1,   4(sp)                                \n"
        "        sw          x2,   8(sp)                                \n"
        "        sw          x3,  12(sp)                                \n"
        "        sw          x5,  16(sp)                                \n"
        "        sw          x6,  20(sp)                                \n"
        "        sw          x7,  24(sp)                                \n"
        "        sw          x8,  28(sp)                                \n"
        "        sw          x9,  32(sp)                                \n"
        "        sw         x10,  36(sp)                                \n"
        "        sw         x11,  40(sp)                                \n"
        "        sw         x12,  44(sp)                                \n"
        "        sw         x13,  48(sp)                                \n"
        "        sw         x14,  52(sp)                                \n"
        "        sw         x15,  56(sp)                                \n"
        "        sw         x16,  60(sp)                                \n"
        "        sw         x17,  64(sp)                                \n"
        "        sw         x18,  68(sp)                                \n"
        "        sw         x19,  72(sp)                                \n"
        "        sw         x20,  76(sp)                                \n"
        "        sw         x21,  80(sp)                                \n"
        "        sw         x22,  84(sp)                                \n"
        "        sw         x23,  88(sp)                                \n"
        "        sw         x24,  92(sp)                                \n"
        "        sw         x25,  96(sp)                                \n"
        "        sw         x26, 100(sp)                                \n"
        "        sw         x27, 104(sp)                                \n"
        "        sw         x28, 108(sp)                                \n"
        "        sw         x29, 112(sp)                                \n"
        "        sw         x30, 116(sp)                                \n"
        "        sw         x31, 120(sp)                                \n");
    Reg id = CPU::mcause();
    if(id & CLINT::INTERRUPT) {
        if((id & IC::INT_MASK) == CLINT::IRQ_MAC_SOFT) {
            IC::ipi_eoi(id & IC::INT_MASK); // is it an IPI? clear
        }
        if((id & IC::INT_MASK) == CLINT::IRQ_MAC_TIMER) {
            Timer::reset(); // is it the timer? clear
            CPU::sie(CPU::STI);
        }
        Reg interrupt_id = 1 << ((id & IC::INT_MASK) - 2);
        if(CPU::int_enabled() && (CPU::sie() & (interrupt_id)))
            CPU::mip(interrupt_id); // forward to supervisor mode
    }
    ASM("        lw          x1,   4(sp)                                \n"
        "        lw          x2,   8(sp)                                \n"
        "        lw          x3,  12(sp)                                \n"
        "        lw          x5,  16(sp)                                \n"
        "        lw          x6,  20(sp)                                \n"
        "        lw          x7,  24(sp)                                \n"
        "        lw          x8,  28(sp)                                \n"
        "        lw          x9,  32(sp)                                \n"
        "        lw         x10,  36(sp)                                \n"
        "        lw         x11,  40(sp)                                \n"
        "        lw         x12,  44(sp)                                \n"
        "        lw         x13,  48(sp)                                \n"
        "        lw         x14,  52(sp)                                \n"
        "        lw         x15,  56(sp)                                \n"
        "        lw         x16,  60(sp)                                \n"
        "        lw         x17,  64(sp)                                \n"
        "        lw         x18,  68(sp)                                \n"
        "        lw         x19,  72(sp)                                \n"
        "        lw         x20,  76(sp)                                \n"
        "        lw         x21,  80(sp)                                \n"
        "        lw         x22,  84(sp)                                \n"
        "        lw         x23,  88(sp)                                \n"
        "        lw         x24,  92(sp)                                \n"
        "        lw         x25,  96(sp)                                \n"
        "        lw         x26, 100(sp)                                \n"
        "        lw         x27, 104(sp)                                \n"
        "        lw         x28, 108(sp)                                \n"
        "        lw         x29, 112(sp)                                \n"
        "        lw         x30, 116(sp)                                \n"
        "        lw         x31, 120(sp)                                \n"
        "        addi        sp, sp,    124                             \n");
    ASM("mret");
}

__END_SYS

void _setup() { Setup_SifiveE::init(); }
