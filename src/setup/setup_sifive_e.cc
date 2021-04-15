// EPOS RISC-V sifive SETUP

#include <utility/ostream.h>
#include <utility/elf.h>
#include <utility/debug.h>
#include <system/info.h>
#include <architecture.h>
#include <machine.h>

using namespace EPOS::S;
typedef unsigned int Reg;

//!P2:
// (both the following are not yet solved)
// _mmode_forward must be rellocated to avoid being erased from MMU::_free

extern "C"
{
    [[gnu::naked, gnu::section(".init")]] void _setup();
    void _print(const char * s) { Display::puts(s); }
    void _panic() { Machine::panic(); }
}

// char placeholder[] = "System_Info placeholder. Actual System_Info will be added by mkbi!";
char placeholder[] = "System_Info placeholder. Actual System_Info will be added by mkbi!_____________________________________________________________________________________________________________________________________________________________________________________________";
System_Info * si;

extern "C" [[gnu::interrupt, gnu::aligned(4)]] void _mmode_forward() {
    Reg id = CPU::mcause();
    if((id & IC::INT_MASK) == CLINT::IRQ_MAC_TIMER) {
        Timer::reset();
        CPU::sie(CPU::STI);
    }
    Reg interrupt_id = 1 << ((id & IC::INT_MASK) - 2);
    if(CPU::int_enabled() && (CPU::sie() & (interrupt_id)))
        CPU::mip(interrupt_id);
}

__BEGIN_SYS
EPOS::S::U::OStream kout, kerr;
char * bi;

class Setup_SifiveE {
private:
    // Physical memory map
    static const unsigned int SYS_INFO = Memory_Map::SYS_INFO;
    static const unsigned int PAGE_TABLES = Memory_Map::PAGE_TABLES;
    static const unsigned int MMODE_F = Memory_Map::MMODE_F;
    static const unsigned int MEM_BASE = Memory_Map::MEM_BASE;
    static const unsigned int MEM_TOP = Memory_Map::MEM_TOP;
    
    typedef CPU::Reg Reg;
    typedef MMU::RV32_Flags RV32_Flags;
    typedef MMU::Page Page;
    typedef MMU::Page_Table Page_Table;
    typedef MMU::Page_Directory Page_Directory;
    typedef MMU::PT_Entry PT_Entry;

public:
    static void init() { setup_machine_environment(); }
    static void setup_machine_environment();
    static void setup_supervisor_environment();
    static void build_page_tables();
    static void clean_bss();
    static void build_lm();
    static void load_parts();
};

void Setup_SifiveE::load_parts()
{
    // Relocate System_Info
    if(sizeof(System_Info) > sizeof(Page)) {
        db<Setup>(ERR) << "System_Info is bigger than a page (" << sizeof(System_Info) << ")!" << endl;
        _panic();
    }
    memcpy(reinterpret_cast<void *>(SYS_INFO), si, sizeof(System_Info));
    
    // Load INIT
    ELF * ini_elf = reinterpret_cast<ELF *>(&bi[si->bm.init_offset]);
    ELF * sys_elf = reinterpret_cast<ELF *>(&bi[si->bm.system_offset]);
    
    if(si->lm.has_ini) {
        db<Setup>(TRC) << "Setup_SifiveE::load_init()" << endl;
        if(ini_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "INIT code segment was corrupted during SETUP!" << endl;
            _panic();
        }
        
        for(int i = 1; i < ini_elf->segments(); i++)
            if(ini_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "INIT data segment was corrupted during SETUP!" << endl;
                _panic();
            }
    }
    
    if((long unsigned int)ini_elf->segment_size(0) > sys_elf->segment_address(0) - ini_elf->segment_address(0)) {
        db<Setup>(ERR) << "init is larger than its reserved memory" << endl;
        _panic();
    } 
    db<Setup>(TRC) << "init has " << hex << sys_elf->segment_address(0) - ini_elf->segment_address(0) - ini_elf->segment_size(0) << " unused bytes of memory" << endl;
    
    // Load SYSTEM
    if(si->lm.has_sys) {
        db<Setup>(TRC) << "Setup_SifiveE::load_system()" << endl;
        if(sys_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "system code segment was corrupted during SETUP!" << endl;
            _panic();
        }
        for(int i = 1; i < sys_elf->segments(); i++)
            if(sys_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "system data segment was corrupted during SETUP!" << endl;
                _panic();
            }
    }
    
    if((long unsigned int)sys_elf->segment_size(0) > sys_elf->segment_address(1) - sys_elf->segment_address(0)) {
        db<Setup>(ERR) << "sys code is larger than its reserved memory" << endl;
        _panic();
    } 
    db<Setup>(TRC) << "sys code has " << hex << sys_elf->segment_address(1) - sys_elf->segment_address(0) - sys_elf->segment_size(0) << " unused bytes of memory" << endl;
    
    if((long unsigned int)ini_elf->segment_size(1) > sys_elf->segment_address(1) + 0x00100000 - sys_elf->segment_address(1)) {
        db<Setup>(ERR) << "init is larger than its reserved memory" << endl;
        _panic();
    } 
    db<Setup>(TRC) << "sys data has " << hex << sys_elf->segment_address(1) + 0x00100000 - sys_elf->segment_address(1) - ini_elf->segment_size(1) << " unused bytes of memory" << endl;
}


void Setup_SifiveE::build_lm()
{
    // Get boot image structure
    si->lm.has_stp = (si->bm.setup_offset != -1u);
    si->lm.has_ini = (si->bm.init_offset != -1u);
    si->lm.has_sys = (si->bm.system_offset != -1u);
    si->lm.has_app = (si->bm.application_offset != -1u);
    si->lm.has_ext = (si->bm.extras_offset != -1u);

    // Check SETUP integrity and get the size of its segments
    si->lm.stp_entry = 0;
    si->lm.stp_segments = 0;
    si->lm.stp_code = ~0U;
    si->lm.stp_code_size = 0;
    si->lm.stp_data = ~0U;
    si->lm.stp_data_size = 0;

    bi = reinterpret_cast<char *>(Traits<Machine>::MEM_BASE); // bi is loaded at MEM_BASE
    if(si->lm.has_stp) {
        ELF * stp_elf = reinterpret_cast<ELF *>(&bi[si->bm.setup_offset]);
        if(!stp_elf->valid()) {
            db<Setup>(ERR) << "SETUP ELF image is corrupted!" << endl;
            _panic();
        }

        si->lm.stp_entry = stp_elf->entry();
        si->lm.stp_segments = stp_elf->segments();
        si->lm.stp_code = stp_elf->segment_address(0);
        si->lm.stp_code_size = stp_elf->segment_size(0);
        if(stp_elf->segments() > 1) {
            for(int i = 1; i < stp_elf->segments(); i++) {
                if(stp_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(stp_elf->segment_address(i) < si->lm.stp_data)
                    si->lm.stp_data = stp_elf->segment_address(i);
                si->lm.stp_data_size += stp_elf->segment_size(i);
            }
        }
    }

    // Check INIT integrity and get the size of its segments
    si->lm.ini_entry = 0;
    si->lm.ini_segments = 0;
    si->lm.ini_code = ~0U;
    si->lm.ini_code_size = 0;
    si->lm.ini_data = ~0U;
    si->lm.ini_data_size = 0;
    if(si->lm.has_ini) {
        ELF * ini_elf = reinterpret_cast<ELF *>(&bi[si->bm.init_offset]);
        if(!ini_elf->valid()) {
            db<Setup>(ERR) << "INIT ELF image is corrupted!" << endl;
            _panic();
        }

        si->lm.ini_entry = ini_elf->entry();
        si->lm.ini_segments = ini_elf->segments();
        si->lm.ini_code = ini_elf->segment_address(0);
        si->lm.ini_code_size = ini_elf->segment_size(0);
        if(ini_elf->segments() > 1) {
            for(int i = 1; i < ini_elf->segments(); i++) {
                if(ini_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(ini_elf->segment_address(i) < si->lm.ini_data)
                    si->lm.ini_data = ini_elf->segment_address(i);
                si->lm.ini_data_size += ini_elf->segment_size(i);
            }
        }
    }

    // Check SYSTEM integrity and get the size of its segments
    si->lm.sys_entry = 0;
    si->lm.sys_segments = 0;
    si->lm.sys_code = ~0U;
    si->lm.sys_code_size = 0;
    si->lm.sys_data = ~0U;
    si->lm.sys_data_size = 0;
    // si->lm.sys_stack = SYS_STACK;
    // si->lm.sys_stack_size = Traits<System>::STACK_SIZE * si->bm.n_cpus;
    if(si->lm.has_sys) {
        ELF * sys_elf = reinterpret_cast<ELF *>(&bi[si->bm.system_offset]);
        if(!sys_elf->valid()) {
            db<Setup>(ERR) << "OS ELF image is corrupted!" << endl;
            _panic();
        }

        si->lm.sys_entry = sys_elf->entry();
        si->lm.sys_segments = sys_elf->segments();
        si->lm.sys_code = sys_elf->segment_address(0);
        si->lm.sys_code_size = sys_elf->segment_size(0);
        if(sys_elf->segments() > 1) {
            for(int i = 1; i < sys_elf->segments(); i++) {
                if(sys_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(sys_elf->segment_address(i) < si->lm.sys_data)
                    si->lm.sys_data = sys_elf->segment_address(i);
                si->lm.sys_data_size += sys_elf->segment_size(i);
            }
        }

        // if(si->lm.sys_code != SYS_CODE) {
        //     db<Setup>(ERR) << "OS code segment address (" << reinterpret_cast<void *>(si->lm.sys_code) << ") does not match the machine's memory map (" << reinterpret_cast<void *>(SYS_CODE) << ")!" << endl;
        //     _panic();
        // }
        // if(si->lm.sys_code + si->lm.sys_code_size > si->lm.sys_data) {
        //     db<Setup>(ERR) << "OS code segment is too large!" << endl;
        //     _panic();
        // }
        // if(si->lm.sys_data != SYS_DATA) {
        //     db<Setup>(ERR) << "OS data segment address (" << reinterpret_cast<void *>(si->lm.sys_data) << ") does not match the machine's memory map (" << reinterpret_cast<void *>(SYS_DATA) << ")!" << endl;
        //     _panic();
        // }
        // if(si->lm.sys_data + si->lm.sys_data_size > si->lm.sys_stack) {
        //     db<Setup>(ERR) << "OS data segment is too large!" << endl;
        //     panic();
        // }
        // if(MMU::page_tables(MMU::pages(si->lm.sys_stack - SYS + si->lm.sys_stack_size)) > 1) {
        //     db<Setup>(ERR) << "OS stack segment is too large!" << endl;
        //     _panic();
        // }
    }

    // Check APPLICATION integrity and get the size of its segments
    si->lm.app_entry = 0;
    si->lm.app_segments = 0;
    si->lm.app_code = ~0U;
    si->lm.app_code_size = 0;
    si->lm.app_data = ~0U;
    si->lm.app_data_size = 0;
    if(si->lm.has_app) {
        ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
        if(!app_elf->valid()) {
            db<Setup>(ERR) << "Application ELF image is corrupted!" << endl;
            _panic();
        }
        si->lm.app_entry = app_elf->entry();
        si->lm.app_segments = app_elf->segments();
        si->lm.app_code = app_elf->segment_address(0);
        si->lm.app_code_size = app_elf->segment_size(0);
        if(app_elf->segments() > 1) {
            for(int i = 1; i < app_elf->segments(); i++) {
                if(app_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(app_elf->segment_address(i) < si->lm.app_data)
                    si->lm.app_data = app_elf->segment_address(i);
                si->lm.app_data_size += app_elf->segment_size(i);
            }
        }
        // if(Traits<System>::multiheap) { // Application heap in data segment
        //     si->lm.app_data_size = MMU::align_page(si->lm.app_data_size);
        //     si->lm.app_stack = si->lm.app_data + si->lm.app_data_size;
        //     si->lm.app_data_size += MMU::align_page(Traits<Application>::STACK_SIZE);
        //     si->lm.app_heap = si->lm.app_data + si->lm.app_data_size;
        //     si->lm.app_data_size += MMU::align_page(Traits<Application>::HEAP_SIZE);
        // }
        // if(si->lm.has_ext) { // Check for EXTRA data in the boot image
        //     si->lm.app_extra = si->lm.app_data + si->lm.app_data_size;
        //     si->lm.app_extra_size = si->bm.img_size - si->bm.extras_offset;
        //     if(Traits<System>::multiheap)
        //         si->lm.app_extra_size = MMU::align_page(si->lm.app_extra_size);
        //     si->lm.app_data_size += si->lm.app_extra_size;
        // }
    }
}

void Setup_SifiveE::build_page_tables()
{
    // Address of the Directory
    Reg page_tables = PAGE_TABLES;
    MMU::_master = new ( (void *) page_tables ) Page_Directory();

    // Number of kernel entries in each directory
    unsigned sys_entries = 512 + MMU::page_tables(MMU::pages(Traits<Machine>::MEM_TOP + 1 - Traits<Machine>::MEM_BASE));

    MMU::_master->remap(page_tables + 4096, RV32_Flags::VALID, 0, sys_entries);

    // Map logical addrs back to themselves; with this, the kernel may access any
    // physical RAM address directly (as if paging wasn't there)
    for(unsigned i = 0; i < sys_entries; i++)
    {
        Page_Table * pt = new ( (void *)(page_tables + 4*1024*(i+1)) ) Page_Table();
        pt->remap(i * 1024*4096, RV32_Flags::SYS);
    }
}

extern "C" char __bss_start;
extern "C" char _end;

void Setup_SifiveE::clean_bss()
{
    unsigned * bss_start = reinterpret_cast<unsigned *>(&__bss_start);
    unsigned * bss_end = reinterpret_cast<unsigned *>(&_end);
    
    db<Setup>(TRC) << "bss_start=" << bss_start << ", bss_end=" << bss_end << endl;
    for (unsigned * word = bss_start; word < bss_end; word++) {
        unsigned * t = new (word) unsigned;
        *t = 0;
    }
}

void Setup_SifiveE::setup_supervisor_environment()
{
    Display::init();
    
    // We must clean the bss before setting MMU::_master
    clean_bss();

    // This creates and configures the kernel page tables (which map logical==physical)
    build_page_tables();

    si = reinterpret_cast<System_Info*>(placeholder);
    build_lm();
    load_parts();

    // forward everything
    CPU::satp((0x1 << 31) | (PAGE_TABLES >> 12));
    
    CPU::sepc_write(si->lm.ini_entry);

    // Interrupts will remain disable until the Context::load at Init_First
    CPU::sstatus_write(CPU::SPP_S);
    CPU::sie_write(CPU::SSI | CPU::STI | CPU::SEI);

    ASM("sret");
}

void Setup_SifiveE::setup_machine_environment()
{
    // We first configure the M-mode CSRs and then switch to S-mode
    // configure paging. After that, we won't return to M-mode; an exception
    // is the forwarding of ints and excps to S-mode.
    CPU::mie_write(CPU::MSI | CPU::MTI | CPU::MEI);
    CPU::mmode_int_disable();

    // We need to set:
    //      MPP_S: to switch to S-mode after mret
    //      MPIE:  otherwise we won't ever receive interrupts
    CPU::mstatus_write(CPU::MPP_S | CPU::MPIE);

    // We store mhartid at tp, since it becomes inaccessible while in S-mode.
    Reg core = CPU::mhartid();
    CPU::tp(core);

    // Set stack for each core
    CPU::sp(Traits<Machine>::BOOT_STACK - Traits<Machine>::STACK_SIZE * core);

    // Guarantee that paging is off before going to S-mode.
    CPU::satp(0);

    // Forward all ints and excs to S-mode.
    //!ECALLS: Not yet implemented.
    CPU::mideleg_write(CPU::SSI | CPU::STI | CPU::SEI);
    CPU::medeleg_write(0xffff);

    // Relocate _mmode_forward - 1024 bytes are enough
    char * src = reinterpret_cast<char *>(&_mmode_forward);
    char * dst = reinterpret_cast<char *>(MMODE_F);
    for(int i=0; i < 1024; i++){
        *dst = *src;
        src++;
        dst++;
    }
    
    // All ints received in M-mode are forwarded to S-mode.
    // The first two bits indicate the mode: Direct or Vectored;
    // we opted for Direct.
    CPU::mtvec(MMODE_F & 0xfffffffc);
    CPU::mepc((unsigned)&setup_supervisor_environment);

    ASM("mret");
}

__END_SYS

void _setup() { Setup_SifiveE::init(); }
