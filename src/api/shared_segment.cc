// EPOS Memory Segment Implementation

#include <memory.h>

__BEGIN_SYS

Shared_Segment::List Shared_Segment::_list; 

// Methods

Shared_Segment::Shared_Segment(unsigned int port, unsigned int bytes, const Flags & flags): Segment(bytes, flags)
{
    tasks = 1;
    db<Segment>(TRC) << "Shared_Segment(bytes=" << bytes << ",flags=" << flags << ") [Chunk::_pt=" << Chunk::pt() << "] => " << this << endl;
}

unsigned int Shared_Segment::get_sseg(unsigned int port) { 
    List::Element * el = _list.find(port);
    // Shared_Segment::_queue.remove_head();
    // kout << sizeof(Queue) << endl;
    kout << "QQQQQQQQQQQQQQQ" << endl;
    kout << el->object()->sseg << endl;
    // kout << this << endl;
    return 0; 
}


// Segment::Segment(unsigned int bytes, const Flags & flags): Chunk(bytes, flags)
// {
//     db<Segment>(TRC) << "Segment(bytes=" << bytes << ",flags=" << flags << ") [Chunk::_pt=" << Chunk::pt() << "] => " << this << endl;
// }


// Segment::~Segment()
// {
//     db<Segment>(TRC) << "~Segment() [Chunk::_pt=" << Chunk::pt() << "]" << endl;
// }


// unsigned int Segment::size() const
// {
//     return Chunk::size();
// }


// Segment::Phy_Addr Segment::phy_address() const
// {
//     return Chunk::phy_address();
// }


// int Segment::resize(int amount)
// {
//     db<Segment>(TRC) << "Segment::resize(amount=" << amount << ")" << endl;

//     return Chunk::resize(amount);
// }

__END_SYS
