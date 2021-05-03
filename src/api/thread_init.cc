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
    
    if(Traits<System>::multitask) {
        char * bi = reinterpret_cast<char*>(Memory_Map::MEM_BASE);
        
        // We need W permission to load the segment
        Segment * code_seg = new (SYSTEM) Segment(si->lm.app_code_size, MMU::Flags::ALL);
        Segment * data_seg = new (SYSTEM) Segment(si->lm.app_data_size, MMU::Flags::UDATA);
        Task * app_task =  new (SYSTEM) Task(code_seg, data_seg);

        db<Setup>(TRC) << "app_task = " << app_task << endl;
        Task::activate(app_task);
        
        // Task::_active->_heap = reinterpret_cast<Heap *>(Memory_Map::APP_HEAP);
        
        // Load App
        if(si->lm.has_app) {
            ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
            db<Setup>(TRC) << "Setup_SifiveE::load_app()" << endl;
            if(app_elf->load_segment(0) < 0) {
                db<Setup>(ERR) << "Application code segment was corrupted during INIT!" << endl;
                Machine::panic();
            }
            for(int j = 1; j < app_elf->segments(); j++)
                if(app_elf->load_segment(j) < 0) {
                    db<Setup>(ERR) << "Application data segment was corrupted during INIT!" << endl;
                    Machine::panic();
                }
        }

        // Load Extra
        db<Setup>(TRC) << "Setup_SifiveE::load_extra()" << endl;
        if(si->lm.has_ext)
            memcpy(reinterpret_cast<void *>(si->lm.app_extra), &bi[si->bm.extras_offset], si->lm.app_extra_size);
        
        new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN), reinterpret_cast<Main *>(si->lm.app_entry));
    }
   
    // Idle thread creation does not cause rescheduling (see Thread::constructor_epilogue)
    Thread * idle = new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::IDLE), Thread::idle);
    //!TODO: There should be two ctors: one for system threads and another for uthreads
    idle->_context->_st |= CPU::SPP_S;

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
