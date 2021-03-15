#include <utility/ostream.h>
#include <time.h>

using namespace EPOS;

OStream cout;

int __attribute__((optimize("O0"))) dummy(int n) {
    long long int a = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a += i*j;
        }
    }
    return a;
}

static unsigned ITERATIONS = 1000;
int main()
{
    // Chronometer chron;
    // unsigned chron_freq = chron.frequency();
    // unsigned alarm_freq = Alarm::frequency();
    // cout << "Chron Frequency: " << chron_freq << endl;
    // cout << "Alarm Frequency: " << alarm_freq << endl;
    // unsigned int RESCALE = chron_freq / alarm_freq;
    // chron.reset();
    // chron.start();
    // for (int i = 0; i < ITERATIONS; i++) {
    //     dummy(900);
    // }
    // chron.stop();
    // unsigned avg = (chron.ticks()/ITERATIONS)/RESCALE;
    // cout << "T=" << avg << endl; 
    cout << "Hello world!" << endl;
    return 0;
}
