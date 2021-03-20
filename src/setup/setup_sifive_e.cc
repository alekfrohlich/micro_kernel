// EPOS RISC-V sifive SETUP

#include <system/config.h>
#include <architecture/cpu.h>
#include <machine/ic.h>

using namespace EPOS::S;
typedef CPU::Reg Reg;

extern "C" 
{
    void _setup() __attribute__ ((used, naked, section(".init")));
    void _int_entry();
    void _start();
    void _wait() 
    { 
        // is it possible that wfi is ignored?
        CPU::halt();
        _start();
    }
}

class PageTable
{
    private:
        typedef unsigned int PTE;

        enum
        {
            V = 1 << 0,
        };

        PTE ptes[1024];

public:
    __attribute__((optimize("O0"))) PageTable ()
    {
        // this>>12 é o número da página desta tabela de páginas
        // | PNN[1] | PNN[0] | OFFSET | -> user address (32 bits)
        // PPN[1] -> directory index || address_page <- page_tables + PNN[1]*4096 é o início de uma tabela de páginas
        // PPN[0] -> address_page index || PTE <- address_page

        for(int i = 0; i < 1024; i++)
        {
            Reg pte = (((unsigned)this - Traits<Machine>::PAGE_TABLES)>>12) - 1;
            pte = pte << 20;
            pte += ((i) << 10);
            pte = pte | V; // R/W/X
            ptes[i] = pte;
        }
    }
    ~PageTable()
    {

    }
};

class PageDirectory
{
private:
        typedef unsigned int PDE;

        enum
        {
            V = 1 << 0,
        };

        PDE pdes[1024];

public:
        __attribute__((optimize("O0"))) PageDirectory()
        {
            Reg page_table = (unsigned)this;
            page_table = page_table >> 12; 

            for (int i = 0; i < 1024; i++) 
            {
                Reg pde = (page_table << 10);

                pde += ((i+1) << 10);
                pde = pde | V;

                pdes[i] = pde;
            }                
        }
        ~PageDirectory();
};

PageTable *page_table_array[1024];

static void build_page_tables() 
{
    Reg page_tables= Traits<Machine>::PAGE_TABLES; // address of the page table root

    new ( (void *) page_tables ) PageDirectory();

    for(int i = 0; i < 1024; i++)
    {
        page_table_array[i] = new ( (void *)(page_tables + 4*1024*(i+1))  ) PageTable();
    }
}

static void setup_supervisor_environment() 
{
    CPU::sstatus_write( CPU::SIE | CPU::SPIE | CPU::SPP_S );
    CPU::sie_write( CPU::SSI | CPU::STI | CPU::SEI );
    // we need a separate handler that doesnt use M registers
    CPU::stvec_write((unsigned)&_int_entry & 0xfffffffe);
    CPU::sepc_write( (unsigned)&_start);
    ASM("sret");
}

static void setup_machine_environment(Reg core)
{
    if (core == 0) {
        build_page_tables();
        // Reg satp = (0x1 << 31) | (Traits<Machine>::PAGE_TABLES >> 12);
        CPU::satp_write(0); // paging off
        CPU::mstatus_write(CPU::MPP_S | CPU::MPIE);
        CPU::mepc((unsigned)&setup_supervisor_environment);
        
        // was done for each hart; why?
        CPU::mtvec((unsigned)&_int_entry & 0xfffffffe);
        // CPU::mie_write( (0 << 3) | (1 << 7) | (0 << 11) );
        CPU::mie_write(CPU::MTI);
        CPU::mideleg_write( CPU::MTI | CPU::STI );
        ASM("mret");
    } else {
        CPU::mstatus_write(CPU::MSTATUS_DEFAULTS);
        CPU::mie(CPU::MSI | CPU::MTI | CPU::MEI);
        CPU::mepc((unsigned)&_wait);
        ASM("mret");
    }
}

void _setup() {
    CPU::int_disable();
    Reg core = CPU::id();
    // set stack for each core
    CPU::sp(Traits<Machine>::BOOT_STACK - Traits<Machine>::STACK_SIZE * core);
    setup_machine_environment(core);
}