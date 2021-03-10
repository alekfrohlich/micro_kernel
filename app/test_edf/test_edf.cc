// Description:
// The EDF time unit is Alarm::Tick
// In the current configuration, we have
//  preemptive = true
//  dynamic = true
//  timed = false
// We've decided to turn off timed preemptions since that increased the time
// system spent in between job execution without any benefits.

// Since letting deadline = NORMAL (the priority) with timed = true makes EDF = RR, we'll
// be switching back to RR after this exercise (for the kernel development).

#include <utility/ostream.h>
#include <real-time.h>

using namespace EPOS;

OStream cout;

// template <NUM_TASKS>
// class Real_Time_System {
//     typedef fptr Job;
//     Job job[NUM_TASKS]
//     Periodic_Thread * task[NUM_TASKS];
//     void init();
// }

int work(int n) {

    while (true) {
        cout << "Task[" << n << "] did some work!" << endl;
        Periodic_Thread::wait_next();
    }
}

static const unsigned DEADLINE[] = {
    150,
    300,
};

Periodic_Thread * task[2];

int main()
{
    // terminal output is important after the following line
    cout << "Testing EDF..." << endl;

    for (int i = 0; i < 2; i++)
        task[i] = new Periodic_Thread(DEADLINE[i], &work, i);

    // main will halt forever
    for (int i = 0; i < 2; i++)
        task[i]->join();

    for (int i = 0; i < 2; i++)
        delete task[i];

    return 0;
}
