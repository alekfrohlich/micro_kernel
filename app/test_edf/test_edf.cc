// Description:
// In the current configuration, we have
//  preemptive = true
//  dynamic = true
//  timed = false
// We've decided to turn off timed preemptions since that increased the time
// system spent in between job execution without any benefits.

// Test scenario:
// Two tasks,
//  A: cap=1000, deadline=2000
//  B: cap=1500, deadline=3000
// CPU_Usage = 100%; to avoid missing deadlines due to system execution, we actualy
// have A and B execute a little bit faster than their caps.
// The idea is to obtain the following timeline:
//
// 0000-1000: A runs and finishes (wait_next())
// 1000-2000: B runs until A's alarm (50 leftover)
// 2000-2500: 300 < 400, so B runs and finishes (wait_next())
// 2500-3000: A runs until B's alarm (50 leftover)
// 3000-3500: 400 < 600, so A runs and finishes (wait_next())
// 3500-4000: B runs until A's alarm (100 leftover)
// Here, in practice, A runs before B due to the reschedule provoked by A's alarm (choose() will insert B at tail and so A will be returned from remove_head())
// 4000-5000: B continues and finishes (wait_next())
// 5000-6000: A runs and finishes (wait_next())
// Then, the system repeats (at every 6000 ticks we reset _priority = _period to avoid overflow)

// Observations:
// The EDF time unit is Alarm::Tick

#include <utility/ostream.h>
#include <machine/display.h>
#include <architecture.h>
#include <real-time.h>

using namespace EPOS;

OStream cout;

static const unsigned DEADLINE[] = {
    2000,
    3000,
};

int __attribute__((optimize("O0"))) work(int n) {
    EPOS::S::CPU::int_enable(); // Join makes so that Task_A starts executing with interrupts disabled.
    cout << "Begin: " << ((n == 0) ? 'A' : 'B') << ", Prio=" << Periodic_Thread::self()->priority() << " [" << Alarm::elapsed() << "]" << endl;
    while (Alarm::elapsed() < 15000) {
        unsigned last_time = Alarm::elapsed();
        unsigned tot = 0;
        while (tot < DEADLINE[n]/2 - 50) { // -50 to allow for system time
            EPOS::S::CPU::int_disable();
            if (Alarm::elapsed() - last_time < 500) // we might be interrupted, so only add if we werent
                tot += Alarm::elapsed() - last_time;
            last_time = Alarm::elapsed();
            EPOS::S::CPU::int_enable();
        }
        cout << "End: " << ((n == 0) ? 'A' : 'B') << ", Prio=" << Periodic_Thread::self()->priority() << " [" << Alarm::elapsed() << "]" << endl;
        Periodic_Thread::wait_next();
        cout << "Begin: " << ((n == 0) ? 'A' : 'B') << ", Prio=" << Periodic_Thread::self()->priority() << " [" << Alarm::elapsed() << "]" << endl;
    }
    return 0;
}

Periodic_Thread * task[2];

int main()
{
    Display::clear();;

    task[0] = new Periodic_Thread(DEADLINE[0], &work, 0);
    task[1] = new Periodic_Thread(DEADLINE[1], &work, 1);

    // main will halt forever
    for (int i = 0; i < 2; i++)
        task[i]->join();

    for (int i = 0; i < 2; i++)
        delete task[i];

    return 0;
}
