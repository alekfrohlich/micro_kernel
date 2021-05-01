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
    Init_Application() {
        db<Init>(TRC) << "Init_Application()" << endl;

        // Initialize Application's heap
        db<Init>(INF) << "Initializing application's heap: " << endl;
        // Application::_heap = new (&Application::_preheap[0]) Heap(APP_HEAP, HEAP_SIZE);
        Application::_heap = new (APP_HEAP) Heap(reinterpret_cast<void*>(Traits<Application>::APP_HEAP+sizeof(Heap)), HEAP_SIZE-sizeof(Heap));
        
        db<Init>(INF) << "done!" << endl;
    }
};

// Global object "init_application"  must be linked to the application (not
// to the system) and there constructed at first.
Init_Application init_application;

__END_SYS
