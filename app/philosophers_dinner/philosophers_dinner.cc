// EPOS Semaphore Component Test Program

#include <machine/display.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>

using namespace EPOS;

const int iterations = 10;

Mutex table;

Thread * phil[5];
Semaphore * chopstick[5];

OStream cout;

int philosopher(void* params);

struct parameters{
public:
  parameters(int _n, int _l, int _c): n(_n), l(_l), c(_c) {}
  int n, l, c;
};

int main()
{
    table.lock();
    Display::clear();
    Display::position(0, 0);
    cout << "The Philosopher's Dinner:" << endl;

    for(int i = 0; i < 5; i++)
        chopstick[i] = new Semaphore;
    parameters p1 = parameters(0,  5, 32);
    parameters p2 = parameters(1, 10, 44);
    parameters p3 = parameters(2, 16, 39);
    parameters p4 = parameters(3, 16, 24);
    parameters p5 = parameters(4, 10, 20);
    phil[0] = new Thread(&philosopher, &p1);
    phil[1] = new Thread(&philosopher, &p2);
    phil[2] = new Thread(&philosopher, &p3);
    phil[3] = new Thread(&philosopher, &p4);
    phil[4] = new Thread(&philosopher, &p5);

    cout << "Philosophers are alive and hungry!" << endl;

    Display::position(7, 44);
    cout << '/';
    Display::position(13, 44);
    cout << '\\';
    Display::position(16, 35);
    cout << '|';
    Display::position(13, 27);
    cout << '/';
    Display::position(7, 27);
    cout << '\\';
    Display::position(19, 0);

    cout << "The dinner is served ..." << endl;
    table.unlock();

    for(int i = 0; i < 5; i++) {
        int ret = phil[i]->join();
        table.lock();
        Display::position(20 + i, 0);
        cout << "Philosopher " << i << " ate " << ret << " times " << endl;
        table.unlock();
    }

    for(int i = 0; i < 5; i++)
        delete chopstick[i];
    for(int i = 0; i < 5; i++)
        delete phil[i];

    cout << "The end!" << endl;

    return 0;
}

int philosopher(void * params)
{
    parameters * parameters_phi = reinterpret_cast<parameters *>(params);
    int n = parameters_phi->n;
    int l = parameters_phi->l;
    int c = parameters_phi->c;
    
    int first = (n < 4)? n : 0;
    int second = (n < 4)? n + 1 : 4;

    for(int i = iterations; i > 0; i--) {

        table.lock();
        Display::position(l, c);
        cout << "thinking";
        table.unlock();

        Alarm::delay(1000000);

        table.lock();
        Display::position(l, c);
        cout << " hungry ";
        table.unlock();

        chopstick[first]->p();   // get first chopstick
        chopstick[second]->p();  // get second chopstick

        table.lock();
        Display::position(l, c);
        cout << " eating ";
        table.unlock();

        Alarm::delay(500000);

        table.lock();
        Display::position(l, c);
        cout << "  sate  ";
        table.unlock();

        chopstick[first]->v();   // release first chopstick
        chopstick[second]->v();  // release second chopstick
    }

    table.lock();
    Display::position(l, c);
    cout << "  done  ";
    table.unlock();

    return iterations;
}
