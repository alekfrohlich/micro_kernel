// EPOS Memory Segment Implementation

#include <system.h>
#include <memory.h>

__BEGIN_SYS

Shared_Segment::List Shared_Segment::_list; 

Shared_Segment::Shared_Segment(unsigned int port, unsigned int bytes, const Flags & flags): Segment(bytes, flags)
{
    db<Segment>(TRC) << "Shared_Segment(bytes=" << bytes << ",flags=" << flags << ") [Chunk::_pt=" << Chunk::pt() << "] => " << this << endl;
    tasks = 1;

    assert(!_list.find(port));
    Shared_Segment_Port * ssport = new (SYSTEM) Shared_Segment_Port(port, this);
    List::Element * ssport_link = new (SYSTEM) List::Element(ssport);
    _list.insert(ssport_link);
}

Shared_Segment * Shared_Segment::get_sseg(unsigned int port) { 
    db<Shared_Segment>(TRC) << "get_sseg(port=" << port << ")" << endl;
    List::Element * ssport_link= _list.find(port);
    if (!ssport_link) {
        return 0;
    }

    return ssport_link->object()->sseg; 
}

__END_SYS
