#include <utility/ostream.h>
#include <utility/handler.h>

#include <process.h>
#include <synchronizer.h>
#include <time.h>

using namespace EPOS;

OStream cout;

int test_thread() {
    Thread::exit(3145);
    return 3145;
}

int suspended_print() {
    cout << "At last!" << endl;
    return '*';
}

Mutex mut;
Semaphore sem(2);

int wait_on_the_mutex() {
    cout << "I gotta go fast!" << endl;
    mut.lock();
    cout << "Ugh that took a while" << endl;
    return 0;
}

int wait_on_the_semaphore(int a , int b, int c) {
    sem.p();
    cout << "Semaphore p, thread = " << a << endl;
    return 0;
}

void alarm_handler() {
    cout << "********************";
    // cout << "The clock is ticking..." << endl;
}

int main()
{    
    cout << "Testing Syscalls!" << endl;
    
    // // Test Thread
    // //!TODO: pass/yield, delete threads
    // cout << "Thread:" << endl;
    // Thread * t = new Thread(&test_thread);
    // //!P3: What should self return?
    // cout << "   t: self=" << t << ", state=" << t->state() << ", priority=" << t->priority() << endl; 
    // cout << "   M: self=" << Thread::self() << ", state=" << Thread::self()->state() << ", priority=" << Thread::self()->priority() << endl;     
    // cout << "t returned with code=" << t->join() << endl;

    // Thread * suspended = new Thread(&suspended_print);
    // suspended->suspend();

    // // Test Synchronization
    // t = new Thread(&wait_on_the_mutex);
    // mut.lock();
    // Alarm::delay(3000000);
    // mut.unlock();
    // t->join();
    
    // Thread * t0 = new Thread(&wait_on_the_semaphore, 0, 0, 0);
    // Thread * t1 = new Thread(&wait_on_the_semaphore, 1, 0, 0);
    // Thread * t2 = new Thread(&wait_on_the_semaphore, 2, 0, 0);    
    // cout << "Let the race begin" << endl;
    // Alarm::delay(3000000);
    // sem.v();
    // t0->join();
    // t1->join();
    // t2->join();

    // Test Time
    // cout << "Time:" << endl;
    // cout << "   frequency=" << Alarm::alarm_frequency() << endl;
    //!P4: Fix the vtable bug
    // Function_Handler handler(&alarm_handler);
    // Alarm(400000, &handler, 5);
    // Alarm::delay(5000000);
    // delete a;

    // Test Chronometer
    // cout << "Chronometer:" << endl;
    // Chronometer * chrono = new Chronometer();
    
    // unsigned long f = chrono->frequency(); //Hertz
    // cout << "frequency = " << f << endl; 
    
    // // Microseconds read = chrono->read();
    // // cout << "read = " << read << endl;
    
    // chrono->start();
    // Alarm::delay(1000000);
    // chrono->stop();
    // unsigned long long ts = chrono->ticks(); //Time Stamp
    // cout << "time_stamp = " << ts << endl; //time_stamp is multiplied by ~12
    
    // chrono->reset();
    // chrono->start();
    // Alarm::delay(500000);
    // chrono->lap();
    // ts = chrono->ticks(); //Time Stamp
    // cout << "time_stamp = " << ts << endl; //time_stamp is multiplied by ~12
    
    // delete chrono;
    
    // Test Display
    // cout << "Display:" << endl;
    // Display::putc('#');
    // Display::puts("Display::puts($%)\n");
    // Alarm::delay(1000000);
    // Display::clear();
    // cout << "Clear\n\n\n" << endl; 
    // int l, c;
    // Display::geometry(&l, &c);
    // cout << "Display::geometry: l=" << l << "  c=" << c << endl; 
    // Display::position(20, 50);
    // Display::putc('#');
    // Display::position(&l, &c);
    // cout << "Display::position: l=" << l << "  c=" << c << endl;
    
    
    
    // Display is not part of the API and so is not tested here (but is used at philosophers dinner)

    // We now wake suspended
    // suspended->resume();

    return 0;
}
