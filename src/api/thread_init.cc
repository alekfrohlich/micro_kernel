// EPOS Thread Initialization

#include <utility/elf.h>
#include <architecture/mmu.h>
#include <machine/timer.h>
#include <machine/ic.h>
#include <system.h>
#include <process.h>
#include <memory.h>


__BEGIN_SYS

extern "C" { void __epos_app_entry(); }

void Thread::init()
{
    db<Init, Thread>(TRC) << "Thread::init()" << endl;
    typedef int (Main)();
    System_Info * si = System::info();
    
    if (!si->lm.has_app) {
        db<Task>(ERR) << "No application was provided!" << endl;
        Machine::panic();
    }

    //!TMP: We need W permission to load the segment; _end might be on code segment if App has no data
    // Task * loader_task = new (SYSTEM) Task( new (SYSTEM) Address_Space(MMU::current()),
    Task * loader_task = new (SYSTEM) Task( new (SYSTEM) Segment(si->lm.app_code_size, MMU::Flags::ALL),
                                            new (SYSTEM) Segment(si->lm.app_data_size, MMU::Flags::UDATA),
                                            reinterpret_cast<Main *>(si->lm.app_entry));
   
   db<Task>(TRC) << loader_task << endl;

    // Load App
    char * bi = reinterpret_cast<char*>(Memory_Map::MEM_BASE);
    ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
    db<Task>(TRC) << "load_app()" << endl;
    if(app_elf->load_segment(0) < 0) {
        db<Task>(ERR) << "Application code segment was corrupted during INIT!" << endl;
        Machine::panic();
    }
    for(int j = 1; j < app_elf->segments(); j++) {
        if(app_elf->segment_type(j) != PT_LOAD)
            continue;
        if(app_elf->load_segment(j) < 0) {
            db<Task>(ERR) << "Application data segment was corrupted during INIT!" << endl;
            Machine::panic();
        }
    }

    // Load Extra
    db<Task>(TRC) << "load_extra()" << endl;
    if(si->lm.has_ext)
        memcpy(reinterpret_cast<void *>(si->lm.app_extra), &bi[si->bm.extras_offset], si->lm.app_extra_size);
    

    // Idle thread creation does not cause rescheduling (see Thread::constructor_epilogue)
    // Thread * idle = new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::IDLE), Thread::idle);
    //!TODO: There should be two ctors: one for system threads and another for uthreads
    // idle->_context->_st |= CPU::SPP_S;

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
