// EPOS RISC-V Memory Map

#ifndef __riscv_memory_map_h
#define __riscv_memory_map_h


#include <system/memory_map.h>


__BEGIN_SYS

struct Memory_Map
{
    static const unsigned int NOT_USED = Traits<Machine>::NOT_USED;
    
    // Memory Mapped stuff
    enum {
        TEST_BASE                   = 0x00100000, // SiFive test engine
        RTC_BASE                    = 0x00101000, // goldfish_rtc
        UART_BASE                   = 0x10000000, // 16550A NS UART
        CLINT_BASE                  = 0x02000000, // Sifive CLINT
        TIMER_BASE                  = 0x02004000, // CLINT timer
        PLIIC_CPU_BASE              = 0x0c000000  // SiFive PLIC
    };

    // Physical Memory
    enum {
        MEM_BASE        = Traits<Machine>::MEM_BASE,
        MEM_TOP         = Traits<Machine>::MEM_TOP,
        MIO_BASE        = Traits<Machine>::MIO_BASE,
        MIO_TOP         = Traits<Machine>::MIO_TOP,
        BOOT_STACK      = Traits<Machine>::BOOT_STACK,
        PAGE_TABLES     = Traits<Machine>::PAGE_TABLES,
    };

    // Logical Address Space
    enum {
        BOOT            = Traits<Machine>::BOOT,
        IMAGE           = Traits<Machine>::IMAGE,
        SETUP           = Traits<Machine>::SETUP,
        INIT            = Traits<Machine>::INIT,

        APP_LOW         = Traits<Machine>::APP_LOW,
        APP_CODE        = Traits<Machine>::APP_CODE,
        APP_DATA        = Traits<Machine>::APP_DATA,
        APP_HIGH        = Traits<Machine>::APP_HIGH,

        PHY_MEM         = Traits<Machine>::PHY_MEM,
        IO              = Traits<Machine>::IO_BASE,

        SYS             = Traits<Machine>::SYS,
        SYS_INFO        = Traits<Machine>::SYS_INFO,
        SYS_CODE        = Traits<Machine>::SYS_CODE,
        SYS_DATA        = Traits<Machine>::SYS_DATA,
        SYS_HEAP        = Traits<Machine>::NOT_USED,
        SYS_STACK       = Traits<Machine>::NOT_USED
    };

    // Logical Address Space
};

__END_SYS

#endif
