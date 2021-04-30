// EPOS RISC V Initialization

#include <machine.h>

__BEGIN_SYS

void Machine::pre_init(System_Info * si)
{
    CPU::stvec_write((unsigned)&IC::entry & 0xfffffffc);
    // Allow system to access user pages
    CPU::sstatus(CPU::SUM);
    IC::init();
}

void Machine::init()
{
    db<Init, Machine>(TRC) << "Machine::init()" << endl;

    if(Traits<Timer>::enabled)
        Timer::init();
}

__END_SYS
