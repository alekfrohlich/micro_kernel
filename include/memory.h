// EPOS Memory Declarations

#ifndef __memory_h
#define __memory_h

#include <architecture.h>
#include <utility/list.h>

__BEGIN_SYS

class Address_Space: private MMU::Directory
{
public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;

public:
    Address_Space();
    Address_Space(MMU::Page_Directory * pd);
    ~Address_Space();

    using MMU::Directory::pd;
    using MMU::Directory::activate;

    Log_Addr attach(Segment * seg);
    Log_Addr attach(Segment * seg, const Log_Addr & addr);
    void detach(Segment * seg);
    void detach(Segment * seg, const Log_Addr & addr);

    Phy_Addr physical(const Log_Addr & address);
};


class Segment: public MMU::Chunk
{
private:
    typedef MMU::Chunk Chunk;

public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef MMU::Flags Flags;

public:

    Segment(unsigned int bytes, const Flags & flags);
    // Segment(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags);
    ~Segment();

    unsigned int size() const;
    Phy_Addr phy_address() const;
    int resize(int amount);
};

class Shared_Segment_Port
{
public:
    Shared_Segment * sseg;
    unsigned int port;
};

// class 

class Shared_Segment: public Segment
{
private:
    typedef MMU::Chunk Chunk;
    
    // Thread Queue
    // typedef Ordered_Queue<Thread, Criterion, Scheduler<Thread>::Element> Queue;
    typedef Port_List<Shared_Segment_Port> List;
    
    // static Shared_Segment * tttest;
    

public:
    typedef CPU::Phy_Addr Phy_Addr;
    typedef MMU::Flags Flags;
    static List _list;
    unsigned int tasks;

public:
    Shared_Segment(unsigned int port, unsigned int bytes, const Flags & flags);
    // Segment(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags);
    // ~Segment();

    // unsigned int size() const;
    // Phy_Addr phy_address() const;
    // int resize(int amount);
    // static unsigned int get_sseg(unsigned int port) { 
    //     Queue::Element * el = _queue.search(port);
    //     kout << "QQQQQQQQQQQQQQQ" << endl;
    //     kout << el->object2();
    //     // kout << this << endl;
    //     return 0; 
    // }
    static unsigned int get_sseg(unsigned int port);
    // unsigned int add_task();
};


__END_SYS

#endif



// // Simple_List_Double
// template<typename T,
//           typename U,
//           typename El = List_Elements::Singly_Linked_Double<T, U>>
// class Simple_List_Double
// {
// public:
//     typedef T Object_Type1;
//     typedef U Object_Type2;
//     typedef El Element;
//     typedef List_Iterators::Forward<El> Iterator;

// public:
//     Simple_List_Double(): _size(0), _head(0), _tail(0) {}

//     bool empty() const { return (_size == 0); }
//     unsigned int size() const { return _size; }

//     Element * head() { return _head; }
//     Element * tail() { return _tail; }

//     Iterator begin() { return Iterator(_head); }
//     Iterator end() { return Iterator(0); }

//     void insert(Element * e) { insert_tail(e); }

//     void insert_head(Element * e) {
//         if(empty())
//             insert_first(e);
//         else {
//             e->next(_head);
//             _head = e;
//             _size++;
//         }
//     }

//     void insert_tail(Element * e) {
//         if(empty())
//             insert_first(e);
//         else {
//             _tail->next(e);
//             e->next(0);
//             _tail = e;
//             _size++;
//         }
//     }

//     Element * remove() { return remove_head(); }

//     Element * remove(Element * e) {
//         if(last())
//             remove_last();
//         else if(e == _head)
//             remove_head();
//         else {
//             Element * p = _head;
//             for(; p && p->next() && (p->next() != e); p = p->next());
//             if(p)
//                 p->next(e->next());
//             if(e == _tail)
//                 _tail = p;
//             _size--;
//         }
//         return e;
//     }

//     Element * remove_head() {
//         if(empty())
//             return 0;
//         if(last())
//             return remove_last();
//         Element * e = _head;
//         _head = _head->next();
//         _size--;
//         return e;
//     }

//     Element * remove_tail() {
//         if(_tail)
//             return remove(_tail);
//         else
//             return 0;
//     }

//     Element * remove(const Object_Type2 obj) {
//         Element * e = search(obj);
//         if(e)
//             return remove(e);
//         return 0;
//     }

//     Element * search(const Object_Type2 obj) {
//         Element * e = _head;
//             for(; e && (e->object2() != obj); e = e->next());
//             return e;
//     }

// protected:
//     bool last() const { return (_size == 1); }

//     void insert(Element * e, Element * p,  Element * n) {
//         p->next(e);
//         e->next(n);
//         _size++;
//     }

//     void insert_first(Element * e) {
//         e->next(0);
//         _head = e;
//         _tail = e;
//         _size++;
//     }

//     Element * remove_last() {
//         Element * e = _head;
//         _head = 0;
//         _tail = 0;
//         _size--;
//         return e;
//     }

// private:
//     unsigned int _size;
//     Element * _head;
//     Element * _tail;
// };


// // Simple List Double Element
//     template<typename T, typename U>
//     class Singly_Linked_Double
//     {
//     public:
//         // typedef T Object_Type;
//         typedef Singly_Linked_Double Element;

//     public:
//         Singly_Linked_Double(const T * o, const U p): _object1(o), _object2(p), _next(0) {}

//         T * object1() const { return const_cast<T *>(_object1); }
//         U object2() const { return _object2; }

//         Element * next() const { return _next; }
//         void next(Element * e) { _next = e; }

//     private:
//         const T * _object1;
//         const U _object2;
//         Element * _next;
//     };