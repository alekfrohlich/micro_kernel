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
            UALL    = VALID | READ | WRITE | EXEC | USR,
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

    public:
        typedef unsigned int PTE;
        PTE ptes[1024];
        Page_Table() {}

        PTE & operator[](unsigned int i) { return ptes[i]; }

        bool map(const RV32_Flags & flags, int from, int to) {
            Phy_Addr * addr = alloc(to - from);
            // Try to alloc contiguous
            if (addr) {
                remap(addr, flags, from , to);
                return true;
            } else {
                for(; from < to; from++){
                    ptes[from] = ((alloc(1) >> 12) << 10) | flags ;
                }
                return false;
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
        
        void print_pt(){
            for(int i=512;i<1024;i++){
                unsigned int ppn = ptes[i] >> 10;
                unsigned int rsw = (ptes[i] << 22) >> 30;
                unsigned int d = (ptes[i] << 24) >> 31;
                unsigned int a = (ptes[i] << 25) >> 31;
                unsigned int g = (ptes[i] << 26) >> 31;
                unsigned int u = (ptes[i] << 27) >> 31;
                unsigned int x = (ptes[i] << 28) >> 31;
                unsigned int w = (ptes[i] << 29) >> 31;
                unsigned int r = (ptes[i] << 30) >> 31;
                unsigned int v = (ptes[i] << 31) >> 31;
                db<MMU>(TRC) << "i = " << i << hex << "ppn = " << ppn <<" rsw =" << rsw;
                db<MMU>(TRC) << " d=" << d << " a=" << a;
                db<MMU>(TRC) << " g=" << g << " u=" << u;
                db<MMU>(TRC) << " x=" << x << " w=" << w;
                db<MMU>(TRC) << " r=" << r << " v=" << v << endl;
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
            _contiguous = _pt->map(_flags, _from, _to);
            if(_contiguous){
                unsigned int ppn =  (*_pt)[_from] >> 10;
                _phy_addr =  ppn << 12;
            } else {
                _phy_addr = 0;
            }
        }

        // Chunk(const Phy_Addr & phy_addr, unsigned int bytes, const Flags & flags)
        // : _from(0), _to(pages(bytes)), _pts(page_tables(_to - _from)), _flags(RV32_Flags(flags)), _pt(calloc(_pts)) {
        //     _pt->remap(phy_addr, flags);
        // }

        // ~Chunk() { free(_phy_addr, _bytes); }
        // ~Chunk() {
        //     for( ; _from < _to; _from++)
        //         free((*static_cast<Page_Table *>(phy2log(_pt)))[_from]);
        //     free(_pt, _pts);
        // }
        ~Chunk() {
            db<Chunk>(TRC) << "Chunk::~(_from=" << _from << " _to=" << _to << ") " << endl;
            if(_contiguous){
                unsigned int ppn =  (*_pt)[_from] >> 10;
                Phy_Addr pt_addr = ppn << 12;
                free(pt_addr, _to - _from);
            }else{
                for( ; _from < _to; _from++){
                    unsigned int ppn =  (*_pt)[_from] >> 10;
                    Phy_Addr pt_addr = ppn << 12;
                    free(pt_addr);
                }
            }
            db<Chunk>(TRC) << "Chunk::~(_pt=" << _pt << " _pts=" << _pts << ") " << endl;
            free(_pt, _pts);
        }

        unsigned int pts() const { return _pts; }
        // Flags flags() const { return Flags(_flags); }
        Page_Table * pt() const { return _pt; }
        unsigned int size() const { return _bytes; }
        Phy_Addr phy_address() const { return _phy_addr; } // always CT
        int resize(unsigned int amount) { return 0; } // no resize in CT

    private:
        unsigned int _from;
        unsigned int _to;
        unsigned int _pts;
        Phy_Addr _phy_addr;
        unsigned int _bytes;
        RV32_Flags _flags;
        Page_Table * _pt;
        bool _contiguous;
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
        
        ~Directory() {
            db<Directory>(TRC) << "Directory::~(_pd=" << _pd << ") " << endl;
            free(_pd);
        }

        Phy_Addr pd() const { return _pd; }

        void activate() {
            CPU::satp((0x1 << 31) | (Phy_Addr)_pd >> 12);
            ASM("sfence.vma");
        }

        Log_Addr attach(const Chunk & chunk, unsigned int from = 0) {
            for(unsigned int i = from; i < PD_ENTRIES - chunk.pts(); i++)
                if(attach(i, chunk.pt(), chunk.pts(), RV32_Flags::VALID))
                    return i << DIRECTORY_SHIFT;
            ASM("sfence.vma"); //P5
            return false;
        }

        // Used to create non-relocatable segments such as code
        Log_Addr attach(const Chunk & chunk, const Log_Addr & addr) {
            unsigned int from = directory(addr);
            if(!attach(from, chunk.pt(), chunk.pts(), RV32_Flags::VALID))
                return Log_Addr(false);
            ASM("sfence.vma"); //P5
            return from << DIRECTORY_SHIFT;
        }

        void detach(const Chunk & chunk) {
            // db<Directory>(TRC) << "Directory::detach(this=" << (void*)this << ", chunk=" << chunk << ")" << endl;
            for(unsigned int i = 0; i < PD_ENTRIES; i++) {
                if (reinterpret_cast<Page_Table*>((*_pd)[i]) == chunk.pt()) {
                    for (unsigned int j = 0; j < chunk.pts(); j++)
                        (*_pd)[i+j] = 0;
                }
            }
            ASM("sfence.vma"); // P5
        }

        void detach(const Chunk & chunk, Log_Addr addr) {
            db<Directory>(TRC) << "Directory::detach(chunk=" << &chunk << " log_addr=" << addr << ") " << endl;
            unsigned int from = directory(addr);
            unsigned int n = chunk.pts();
            db<Directory>(TRC) << "Directory::detach(from=" << from << " n=" << n << ") " << endl;
            for(unsigned int i = from; i < from + n; i++){
                (*_pd)[i] = 0;
            }
            ASM("sfence.vma"); // P5
        }

        Phy_Addr physical(Log_Addr addr) { 
            unsigned int vpnr = addr >> 22;
            unsigned int vpnl = (addr << 10) >> 22;
            unsigned int offset = (addr << 20) >> 20;
            
            unsigned int ppn =  (*_pd)[vpnr] >> 10;
            Page_Table * pt_addr = reinterpret_cast<Page_Table *>(ppn << 12);
            
            unsigned ppn2 = (*pt_addr)[vpnl] >> 10;
            // Phy_Addr phy_addr = reinterpret_cast<Phy_Addr>((ppn2 << 12) + offset);
            Phy_Addr phy_addr = Phy_Addr((ppn2 << 12) + offset);
            
            return phy_addr; 
        }

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
