// EPOS Thread Initialization

#include <utility/elf.h>
#include <machine/timer.h>
#include <machine/ic.h>
#include <system.h>
#include <process.h>
#include <memory.h>

//!P2:  We can use MMU::flags instead
#include <architecture/mmu.h>


__BEGIN_SYS

extern "C" { void __epos_app_entry(); }

void Thread::init()
{
    db<Init, Thread>(TRC) << "Thread::init()" << endl;

    typedef int (Main)();

    System_Info * si = System::info();
    Main * main;

    if(Traits<System>::multitask) {
        main = reinterpret_cast<Main *>(si->lm.app_entry);
        
        // Should we store this somewhere?
        char * bi = reinterpret_cast<char*>(0x80000000);
        Segment * code_seg = new (SYSTEM) Segment(64*4096, MMU::Flags::ALL); // we need W permission to load the segment
        //!P2: do the ctor of Chunk and load_segment interact well if len=0 segments?
        Segment * data_seg = new (SYSTEM) Segment(4096, MMU::Flags::ALL); // UNUSED AS OF NOW
        Address_Space * master = new (SYSTEM) Address_Space(MMU::current());
        new (SYSTEM) Task(master, code_seg, data_seg);
        ASM("sfence.vma");

        // Load APP
        if(si->lm.has_app) {
            ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
            db<Setup>(TRC) << "Setup_SifiveE::load_app()" << endl;
            if(app_elf->load_segment(0) < 0) {
                db<Setup>(ERR) << "Application code segment was corrupted during INIT!" << endl;
                Machine::panic();
            }
            for(int i = 1; i < app_elf->segments(); i++)
                if(app_elf->load_segment(i) < 0) {
                    db<Setup>(ERR) << "Application data segment was corrupted during INIT!" << endl;
                    Machine::panic();
                }
        }
    }
    else {
        // If EPOS is a library, then adjust the application entry point to __epos_app_entry,
        // which will directly call main(). In this case, _init will have already been called,
        // before Init_Application to construct MAIN's global objects.
        main = reinterpret_cast<Main *>(__epos_app_entry);
    }

    new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN), main);

    // Idle thread creation does not cause rescheduling (see Thread::constructor_epilogue)
    new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::IDLE), &Thread::idle);

    // The installation of the scheduler timer handler does not need to be done after the
    // creation of threads, since the constructor won't call reschedule() which won't call
    // dispatch that could call timer->reset()
    // Letting reschedule() happen during thread creation is also harmless, since MAIN is
    // created first and dispatch won't replace it nor by itself neither by IDLE (which
    // has a lower priority)
    if(Criterion::timed)
        _timer = new (SYSTEM) Scheduler_Timer(QUANTUM, time_slicer);

    // No more interrupts until we reach init_first
    CPU::int_disable();

    // Transition from CPU-based locking to thread-based locking
    This_Thread::not_booting();
}

__END_SYS
