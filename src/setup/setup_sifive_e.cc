// EPOS RISC-V sifive SETUP

#include <system/config.h>
#include <architecture/cpu.h>
#include <machine/ic.h>

using namespace EPOS::S;
typedef CPU::Reg Reg;

extern "C" {
    void _setup() __attribute__ ((used, naked, section(".init")));
    void _int_entry();
    void _start();
    void _wait() {
        CPU::halt();
        _start();
    }
}

void _setup()
{
    // j reset?
    CPU::int_disable();
    // disable paging
    CPU::satp(0);
    // config 
    CPU::mtvec((unsigned)&_int_entry & 0xfffffffe);
    // set stack for each core
    Reg core = CPU::id();
    CPU::sp(Traits<Machine>::BOOT_STACK - Traits<Machine>::STACK_SIZE * core);
    if (core == 0) {
        CPU::mstatus_write(CPU::MPP | CPU::MPIE);
        CPU::mepc((unsigned)&_start);
        ASM("mret");
    } else {
        CPU::mstatus_write(CPU::MSTATUS_DEFAULTS);
        CPU::mie(CPU::MSI | CPU::MTI | CPU::MEI);
        CPU::mepc((unsigned)&_wait);
        ASM("mret");
    }
}
