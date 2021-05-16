// EPOS Memory Declarations

#ifndef __memory_h
#define __memory_h

#include <architecture.h>
#include <utility/list.h>

__BEGIN_SYS



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
    Shared_Segment_Port(unsigned int p, Shared_Segment * s) : port(p), sseg(s) {}
    unsigned int port;
    Shared_Segment * sseg;
};

class Shared_Segment: public Segment
{
private:
    typedef MMU::Chunk Chunk;

public:
    typedef Port_List<Shared_Segment_Port> List;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef MMU::Flags Flags;

public:
    Shared_Segment(unsigned int port, unsigned int bytes, const Flags & flags);
    static Shared_Segment * get_sseg(unsigned int port);
    static List _list;
    unsigned int _tasks;
    unsigned int _port;

    //!TODO: Using ...
protected:

};

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

__END_SYS

#endif
