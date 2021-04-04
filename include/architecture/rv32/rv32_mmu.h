// EPOS-- RISC-V 32 MMU Mediator Declarations

#ifndef __riscv32_mmu_h
#define __riscv32_mmu_h

#include <system/memory_map.h>
#include <utility/string.h>
#include <utility/list.h>
#include <utility/debug.h>
#include <architecture/cpu.h>
#include <architecture/mmu.h>

__BEGIN_SYS

class MMU: public MMU_Common<10, 10, 12>
{
    friend class CPU;
    friend class Setup_SifiveE;
private:
    typedef Grouping_List<Frame> List;
    static const unsigned int PHY_MEM = Memory_Map::PHY_MEM;

public:
    // Page Flags
    class RV32_Flags
    {
    public:
        enum {
            VALID    = 1 << 0,
            READ     = 1 << 1,
            WRITE    = 1 << 2,
            EXEC     = 1 << 3,
            USR      = 1 << 4,
            ACCESSED = 1 << 6,
            DIRTY    = 1 << 7,
            SYS      = VALID | READ | WRITE | EXEC,
            KCODE    = VALID | READ | EXEC,
            KDATA    = VALID | READ | WRITE,
            UCODE    = VALID | READ | EXEC | USR,
            UDATA    = VALID | READ | WRITE | USR,
        };

        RV32_Flags() {}
        RV32_Flags(const RV32_Flags & f): _flags(f) {}
        RV32_Flags(unsigned int f): _flags(f) {}
        RV32_Flags(const Flags & f): _flags(((f & Flags::PRE)  ? VALID : 0) | 
                                            ((f & Flags::RW)   ? (READ | WRITE) : READ) |
                                            ((f & Flags::USR)  ? USR : 0) | 
                                            ((f & Flags::EXEC) ? EXEC : 0)) {}
        operator unsigned int() const { return _flags; }

    private:
        unsigned int _flags;

    };

    // Page_Table
    class Page_Table
    {

    friend class Setup_SifiveE;

    private:
        typedef unsigned int PTE;
        PTE ptes[1024];

    public:
        Page_Table() {}

        PTE & operator[](unsigned int i) { return ptes[i]; }

        void map(const RV32_Flags & flags, int from, int to) {
            Phy_Addr * addr = alloc(to - from);
            // Try to alloc contiguous
            if (addr) {
                remap(addr, flags, from , to);
            } else {
                for(; from < to; from++){
                    ptes[from] = ((alloc(1) >> 12) << 10) | flags ;
                }
            }
        }

        void remap(Phy_Addr phy_addr, const RV32_Flags & flags, int from = 0, int to = 1024) {
            for(int i = from; i < to; i++) {
                unsigned int pte = phy_addr >> 12;
                pte = pte << 10;
                pte += ((i) << 10);
                pte = pte | flags;
                ptes[i] = pte;
            }
        }
    };

    // Chunk (for Segment)
    class Chunk
    {
    public:
        Chunk() {}
        // Chunk(unsigned int bytes, Flags flags): _phy_addr(alloc(bytes)), _bytes(bytes), _flags(flags) {}
        // Chunk(Phy_Addr phy_addr, unsigned int bytes, Flags flags): _phy_addr(phy_addr), _bytes(bytes), _flags(flags) {}

        Chunk(unsigned int bytes, const Flags & flags)
        : _from(0), _to(pages(bytes)), _pts(page_tables(_to - _from)), _bytes(bytes), _flags(RV32_Flags(flags)), _pt(calloc(_pts)) {
            _pt->map(_flags, _from, _to);
        }

        // Chunk(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags)
        // : _from(0), _to(pages(bytes)), _pts(page_tables(_to - _from)), _flags(RV32_Flags(flags)), _pt(calloc(_pts)) {
        //     _pt->remap(phy_addr, flags);
        // }

        // ~Chunk() { free(_phy_addr, _bytes); }
        ~Chunk() {
            for( ; _from < _to; _from++)
                free((*static_cast<Page_Table *>(phy2log(_pt)))[_from]);
            free(_pt, _pts);
        }

        unsigned int pts() const { return _pts; }
        // Flags flags() const { return Flags(_flags); }
        Page_Table * pt() const { return _pt; }
        unsigned int size() const { return _bytes; }
        // Phy_Addr phy_address() const { return _phy_addr; } // always CT
        int resize(unsigned int amount) { return 0; } // no resize in CT

    private:
        unsigned int _from;
        unsigned int _to;
        unsigned int _pts;
        // Phy_Addr _phy_addr;
        unsigned int _bytes;
        RV32_Flags _flags;
        Page_Table * _pt;

    };

    // Page Directory
    typedef Page_Table Page_Directory;

    // Directory (for Address_Space)
    class Directory
    {
    public:
        Directory() : _pd(calloc(1)) {
            for(unsigned int i = 0; i < 544; i++){
                (*_pd)[i] = (*_master)[i];
            }
        }
        Directory(Page_Directory * pd) : _pd(pd) {}

        Phy_Addr pd() const { return _pd; }

        void activate() {}

        Log_Addr attach(const Chunk & chunk, unsigned int from = 0) {
            for(unsigned int i = from; i < PD_ENTRIES - chunk.pts(); i++)
                if(attach(i, chunk.pt(), chunk.pts(), RV32_Flags::VALID))
                    return i << DIRECTORY_SHIFT;
            return false;
        }

        // Used to create non-relocatable segments such as code
        Log_Addr attach(const Chunk & chunk, const Log_Addr & addr) {
            unsigned int from = directory(addr);
            if(!attach(from, chunk.pt(), chunk.pts(), RV32_Flags::VALID))
                return Log_Addr(false);
            return from << DIRECTORY_SHIFT;
        }

        void detach(const Chunk & chunk) {}
        void detach(const Chunk & chunk, Log_Addr addr) {}

        Phy_Addr physical(Log_Addr addr) { return addr; }

    private:
        bool attach(unsigned int from, const Page_Table * pt, unsigned int n, RV32_Flags flags) {
            for(unsigned int i = from; i < from + n; i++)
                if((*static_cast<Page_Directory *>(phy2log(_pd)))[i]) // it has already been used
                    return false;
            for(unsigned int i = from; i < from + n; i++, pt++)
                (*static_cast<Page_Directory *>(phy2log(_pd)))[i] = ((Phy_Addr(pt) >> 12) << 10) | flags; // is pt the correct value?
            return true;
        }

    private:
        Page_Directory * _pd;
    };

public:
    MMU() {}

    static Phy_Addr alloc(unsigned int frames = 1) {
        Phy_Addr phy(false);
        if(frames) {
            List::Element * e = _free.search_decrementing(frames);
            if(e)
                phy = reinterpret_cast<unsigned int>(e->object()) + e->size() * PAGE_SIZE;
            else
                db<MMU>(ERR) << "MMU::alloc() failed!" << endl;
        }
        db<MMU>(TRC) << "MMU::alloc(frames=" << frames << ") => " << phy << endl;

        return phy;
    };

    static Phy_Addr calloc(unsigned int frames = 1) {
        Phy_Addr phy = alloc(frames);
        memset(phy2log(phy), 0, frames*PAGE_SIZE);
        return phy;
    }

    static void free(Phy_Addr frame, unsigned int n = 1) {
        db<MMU>(TRC) << "MMU::free(frame=" << frame << ",n=" << n << ")" << endl;

        if(frame && n) {
            List::Element * e = new (frame) List::Element(frame, n);
            List::Element * m1, * m2;
            _free.insert_merging(e, &m1, &m2);
        }
    }

    static unsigned int allocable() { return _free.head() ? _free.head()->size() : 0; }

    static Page_Directory * volatile current() { return _master; }

    static Phy_Addr physical(Log_Addr addr) { return addr; }

    static void flush_tlb() {}
    static void flush_tlb(Log_Addr addr) {}

private:
    static void init();

    static Log_Addr phy2log(const Phy_Addr & phy) { return phy ; }

private:
    static List _free;
    static Page_Directory * _master;
};

__END_SYS

#endif
