// EPOS Application Initializer

#include <architecture.h>
#include <utility/heap.h>
#include <machine.h>
#include <system.h>

extern "C" char _end; // defined by GCC

__BEGIN_SYS

class Init_Application
{
private:
    void * APP_HEAP = reinterpret_cast<void*>(Traits<Application>::APP_HEAP);
    static const unsigned int HEAP_SIZE = Traits<Application>::HEAP_SIZE;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;

public:
    [[gnu::naked]] Init_Application() {
        //!P4: temporary
        // ASM("li sp, 0xff9");
        // ASM("slli sp, 20");
        // ASM("li sp, 0xff900000");
        db<Init>(TRC) << "Init_Application()" << endl;

        // Initialize Application's heap
        db<Init>(INF) << "Initializing application's heap: " << endl;
        // if(Traits<System>::multiheap) { // heap in data segment arranged by SETUP
            // Application::_heap = new (&Application::_preheap[0]) Heap(MMU::alloc(MMU::pages(HEAP_SIZE)), HEAP_SIZE);
        // } else {
        //     for(unsigned int frames = MMU::allocable(); frames; frames = MMU::allocable())
        //         System::_heap->free(MMU::alloc(frames), frames * sizeof(MMU::Page));
        // }
        Application::_heap = new (&Application::_preheap[0]) Heap(APP_HEAP, HEAP_SIZE);
        db<Init>(INF) << "done!" << endl;
    }
};

// Global object "init_application"  must be linked to the application (not
// to the system) and there constructed at first.
Init_Application init_application;

__END_SYS
