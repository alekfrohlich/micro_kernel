// EPOS Thread Component Declarations

#ifndef __process_h
#define __process_h

#include <architecture.h>
#include <machine.h>
#include <utility/queue.h>
#include <utility/handler.h>
#include <scheduler.h>

extern "C" { void __exit(); }

__BEGIN_SYS

class Thread
{
    friend class Init_First;            // context->load()
    friend class Init_System;           // for init() on CPU != 0
    friend class Scheduler<Thread>;     // for link()
    friend class Synchronizer_Common;   // for lock() and sleep()
    friend class Alarm;                 // for lock()
    friend class System;                // for init()
    friend class IC;                    // for link() for priority ceiling

protected:
    static const bool preemptive = Traits<Thread>::Criterion::preemptive;
    static const bool reboot = Traits<System>::reboot;

    static const unsigned int QUANTUM = Traits<Thread>::QUANTUM;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;

public:
    // Thread State
    enum State {
        RUNNING,
        READY,
        SUSPENDED,
        WAITING,
        FINISHING
    };

    // Thread Scheduling Criterion
    typedef Traits<Thread>::Criterion Criterion;
    enum {
        HIGH    = Criterion::HIGH,
        NORMAL  = Criterion::NORMAL,
        LOW     = Criterion::LOW,
        MAIN    = Criterion::MAIN,
        IDLE    = Criterion::IDLE
    };

    // Thread Queue
    typedef Ordered_Queue<Thread, Criterion, Scheduler<Thread>::Element> Queue;

    // Thread Configuration
    struct Configuration {
        Configuration(const State & s = READY, const Criterion & c = NORMAL, Task * t = 0, unsigned int ss = STACK_SIZE)
        : state(s), criterion(c), task(t), stack_size(ss) {}

        State state;
        Criterion criterion;
        Task * task;
        unsigned int stack_size;
    };


public:
    template<typename ... Tn>
    Thread(int (* entry)(Tn ...), Tn ... an);
    template<typename ... Tn>
    Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an);
    ~Thread();

    const volatile State & state() const { return _state; }

    const volatile Priority & priority() const { return _link.rank(); }
    void priority(const Priority & p);

    int join();
    void pass();
    void suspend();
    void resume();

    static Thread * volatile self() { return running(); }
    static void yield();
    static void exit(int status = 0);

protected:
    void constructor_prologue(unsigned int stack_size);
    void constructor_epilogue(const Log_Addr & entry, unsigned int stack_size);

    Criterion & criterion() { return const_cast<Criterion &>(_link.rank()); }
    Queue::Element * link() { return &_link; }

    static Thread * volatile running() { return _scheduler.chosen(); }

    static void lock() { CPU::int_disable(); }
    static void unlock() { CPU::int_enable(); }
    static bool locked() { return CPU::int_disabled(); }

    static void sleep(Queue * q);
    static void wakeup(Queue * q);
    static void wakeup_all(Queue * q);

    static void reschedule();
    static void time_slicer(IC::Interrupt_Id interrupt);

    static void dispatch(Thread * prev, Thread * next, bool charge = true);

    static int idle();

private:
    static void init();

protected:
    char * _stack;
    Segment * _ustack;
    
    Context * volatile _context;
    volatile State _state;
    
    Task * _task;
    
    Queue * _waiting;
    Thread * volatile _joining;
    Queue::Element _link;
    
    static volatile unsigned int _thread_count;
    static Scheduler_Timer * _timer;
    static Scheduler<Thread> _scheduler;
};

// A Java-like Active Object
class Active: public Thread
{
public:
    Active(): Thread(Configuration(Thread::SUSPENDED), &entry, this) {}
    virtual ~Active() {}

    virtual int run() = 0;

    void start() { resume(); }

private:
    static int entry(Active * runnable) { return runnable->run(); }
};


// An event handler that triggers a thread (see handler.h)
class Thread_Handler : public Handler
{
public:
    Thread_Handler(Thread * h) : _handler(h) {}
    ~Thread_Handler() {}

    void operator()() { _handler->resume(); }

private:
    Thread * _handler;
};


class Task
{
    friend class Thread;    // for the running Thread ctor
private:
    static const bool multitask = Traits<System>::multitask;
    typedef CPU::Log_Addr Log_Addr;

protected:
    Task(Address_Space * as, Segment * cs, Segment * ds, int (* entry)())
    : _as(as), _cs(cs), _ds(ds), _code(_as->attach(_cs, Memory_Map::APP_CODE)), _data(_as->attach(_ds, Memory_Map::APP_DATA)) {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds <<  ",code=" << _code << ",data=" << _data << ") => " << this << endl;
        activate(this);
        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN, this), entry);
    }

public:
    Task(Segment * cs, Segment * ds)
    : _as (new (SYSTEM) Address_Space), _cs(cs), _ds(ds), _code(_as->attach(_cs, Memory_Map::APP_CODE)), _data(_as->attach(_ds, Memory_Map::APP_DATA)) {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds <<  ",code=" << _code << ",data=" << _data << ") => " << this << endl;
    }
    
    ~Task(){
        _as->detach(_cs, Memory_Map::APP_CODE);
        _as->detach(_ds, Memory_Map::APP_DATA);
        delete _cs;
        delete _ds;
        delete _as;
    }
    
    static void activate(volatile Task * t) {
        Task::_active = t;
        t->_as->activate();
    }
    
    static unsigned int get_active_pd() {
        return Task::_active->_as->pd();
    }

    static Task * active() {
        return const_cast<Task*>(_active);
    }
    
    Address_Space * address_space() const { return _as; }

    Segment * code_segment() const { return _cs; }
    Segment * data_segment() const { return _ds; }

    Log_Addr code() const { return _code; }
    Log_Addr data() const { return _data; }


private:
    Address_Space * _as;
    Segment * _cs;
    Segment * _ds;
    Log_Addr _code;
    Log_Addr _data;

    Thread * _main;
    static volatile Task * _active;
};

//!TODO: What is the usage of this ctor?
// template<typename ... Tn>
// inline Thread::Thread(int (* entry)(Tn ...), Tn ... an)
// : _state(READY), _waiting(0), _joining(0), _link(this, NORMAL)
// {
//     constructor_prologue(STACK_SIZE);
//     _context = CPU::init_stack(0, _stack + STACK_SIZE, &__exit, entry, an ...);
//     constructor_epilogue(entry, STACK_SIZE);
// }

template<typename ... Tn>
inline Thread::Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an)
: _state(conf.state), _task(conf.task? conf.task : Task::active()), _waiting(0), _joining(0), _link(this, conf.criterion)
{
    constructor_prologue(conf.stack_size);
    _ustack = new (SYSTEM) Segment(Traits<Machine>::STACK_SIZE, MMU::Flags::ALL);
    auto usp = Task::active()->address_space()->attach(_ustack);
    db<Thread>(WRN) << "usp=" << usp << endl;
    usp = CPU::init_user_stack(usp+Traits<Machine>::STACK_SIZE, &__exit, an ...);
    db<Thread>(WRN) << "usp=" << usp << endl;    
    Task::active()->address_space()->detach(_ustack);
    db<Thread>(WRN) << _task->address_space()->attach(_ustack) << endl;

    _context = CPU::init_stack(usp, _stack + conf.stack_size, &__exit, entry, an ...);
    constructor_epilogue(entry, conf.stack_size);
}


__END_SYS

#endif
