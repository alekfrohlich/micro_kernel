#include <system.h>
#include <utility/ostream.h>
#include <process.h>
#include <time.h>
#include <real-time.h>

using namespace EPOS;

OStream cout;

typedef unsigned int Tick;

int function_periodic_a(){
    // CPU::int_enable_for_test(); // ints were disabled
    for(int i=0;i<5;i++){
        Tick begin = Alarm::elapsed();
        Tick last = Alarm::elapsed();
        Tick tot = 0;
        cout << "============ begin a tick: " << begin << endl;
        while(tot <= 100){
            CPU::int_disable();
            if (Alarm::elapsed() - last > 5) {
                last = Alarm::elapsed();
                cout << "============ a is back tick: " << Alarm::elapsed() << endl;
            }
            tot += Alarm::elapsed() - last;
            last = Alarm::elapsed();
            CPU::int_enable();
        }
        cout << "============ end a tick: " << Alarm::elapsed() << endl;
        Periodic_Thread::wait_next();
    }
    return 0;
}

int function_periodic_b(){
    // CPU::int_enable_for_test();
    for(int i=0;i<5;i++){
        Tick begin = Alarm::elapsed();
        Tick last = Alarm::elapsed();
        Tick tot = 0;
        cout << "============ begin b tick: " << begin << endl;
        while(tot <= 1000){
            CPU::int_disable();
            if (Alarm::elapsed() - last > 5) {
                last = Alarm::elapsed();
                cout << "============ b is back tick: " << Alarm::elapsed() << endl;
            }
            tot += Alarm::elapsed() - last;
            last = Alarm::elapsed();
            CPU::int_enable();
        }
        cout << "============ end b tick: " << Alarm::elapsed() << endl;
        Periodic_Thread::wait_next();
    }
    return 0;
}


int main()
{
    cout << "\n\n\n\n============ Hello world!" << endl;


    Periodic_Thread * a = new Periodic_Thread(Periodic_Thread::Configuration(Alarm::timer_period() * 500, 5) , &function_periodic_a);
    Periodic_Thread * b = new Periodic_Thread(Periodic_Thread::Configuration(Alarm::timer_period() * 1600, 5) , &function_periodic_b);

    a->join();
    b->join();
    cout << "============ Goodbye world!\n\n\n\n" << endl;

    return 0;
}