// EPOS RISC-V Timer Mediator Initialization

#include <architecture/cpu.h>
#include <machine/timer.h>
#include <machine/ic.h>

__BEGIN_SYS

void Timer::init()
{
    db<Init, Timer>(TRC) << "Timer::init()" << endl;
    // db<Init, Timer>(TRC) << "Time=" << CPU::rdtime() << endl; // Doesn't work in some versions of QEMU

    CPU::int_disable();

    if(!Traits<System>::multicore || (CPU::id() == 0))
        IC::int_vector(IC::INT_SYS_TIMER, int_handler);

    config(FREQUENCY);
    IC::enable(IC::INT_SYS_TIMER);
}

__END_SYS
