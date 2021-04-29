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
        
        for(unsigned i = 0; i < si->bm.n_apps; i++) {
            // We need W permission to load the segment
            Segment * code_seg = new (SYSTEM) Segment(si->lm.app[i].app_code_size, MMU::Flags::ALL);
            // Segment * code_seg = new (SYSTEM) Segment(1024*1024*4 - 1, MMU::Flags::ALL);
            // Segment * data_seg = new (SYSTEM) Segment(si->lm.app[i].app_data_size, MMU::Flags::ALL); //!P3
            Segment * data_seg = new (SYSTEM) Segment(8*1024*1024, MMU::Flags::UDATA); //P333
            Task * app_task =  new (SYSTEM) Task(code_seg, data_seg);

            
            db<Setup>(TRC) << "app_task = " << hex << app_task << endl;
            Task::activate(app_task);
            
            Task::_active->_heap = reinterpret_cast<Heap *>(Memory_Map::APP_HEAP);
            
            if(si->lm.has_app) {
                ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset[i]]);
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
            
            new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN), reinterpret_cast<Main *>(si->lm.app[i].app_entry));
            // unsigned int satp = CPU::satp();
            // unsigned int pdp = satp << 12;
            // MMU::Page_Table * pdp_ = reinterpret_cast< MMU::Page_Table *>(pdp);
            // pdp_->print_pt(); 
            
            // unsigned int leaf_pt_ppn = pdp_->ptes[1022];
            // unsigned int leaf_pt = (leaf_pt_ppn >> 10) << 12;
            // db<Setup>(TRC) << "leaf_pt = " << hex << leaf_pt << endl;
            // MMU::Page_Table * leaf_ptp_ = reinterpret_cast< MMU::Page_Table *>(leaf_pt);
            // leaf_ptp_->print_pt();
        }
        
        db<Setup>(TRC) << "!!!!!!!!!!!!!!!!!!!!!!!\n\n!!!!!!!!!!!!!!!" << endl;
        // We need to be in the AS of the first thread.
        Task::activate(Thread::self()->_task);
    }
   
    // Idle thread creation does not cause rescheduling (see Thread::constructor_epilogue)
    // new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::IDLE), &Thread::idle);

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
