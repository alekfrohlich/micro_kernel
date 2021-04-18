// EPOS RISC-V Common Run-Time System Information

#ifndef __riscv_sifive_e_info_h
#define __riscv_sifive_e_info_h

#include <system/info.h>

__BEGIN_SYS


struct App_Load_Map
{
private:
    typedef unsigned int LAddr;
    typedef unsigned int PAddr;
    typedef unsigned int Size;

public:
    LAddr app_entry;
    Size  app_segments;
    LAddr app_code;
    Size  app_code_size;
    LAddr app_data;
    LAddr app_stack;
    LAddr app_heap;
    Size  app_data_size;
};

struct System_Info
{
private:
    typedef unsigned int LAddr;
    typedef unsigned int PAddr;
    typedef unsigned int Size;

public:
    // The information we have at boot time (built by MKBI)
    // Modifications to this map requires adjustments at MKBI
    struct Boot_Map
    {
        volatile unsigned int n_cpus;     // Number of CPUs in SMPs
        PAddr mem_base;                   // Memory base address
        PAddr mem_top;                    // Memory top address
        PAddr mio_base;                   // Memory-mapped I/O base address
        PAddr mio_top;                    // Memory-mapped I/O top address
        int node_id;                      // Local node id in SAN (-1 => RARP)
        int space_x;                      // Spatial coordinates of a node (-1 => mobile)
        int space_y;                      //
        int space_z;                      //
        unsigned char uuid[8];            // EPOS image Universally Unique Identifier
        Size img_size;                    // Boot image size (in bytes)
        Size setup_offset;                // Image offsets (-1 => not present)
        Size init_offset;
        Size system_offset;
        Size application_offset[8];
        unsigned int n_apps;
        Size extras_offset;
    };

    struct Load_Map
    {
        bool  has_stp;
        bool  has_ini;
        bool  has_sys;
        bool  has_app;
        bool  has_ext;
        LAddr stp_entry;
        Size  stp_segments;
        LAddr stp_code;
        Size  stp_code_size;
        LAddr stp_data;
        Size  stp_data_size;
        LAddr ini_entry;
        Size  ini_segments;
        LAddr ini_code;
        Size  ini_code_size;
        LAddr ini_data;
        Size  ini_data_size;
        LAddr sys_entry;
        Size  sys_segments;
        LAddr sys_code;
        Size  sys_code_size;
        LAddr sys_data;
        Size  sys_data_size;
        LAddr sys_stack;
        Size  sys_stack_size;
        App_Load_Map app[8];
        // LAddr app_entry;
        // Size  app_segments;
        // LAddr app_code;
        // Size  app_code_size;
        // LAddr app_data;
        // LAddr app_stack;
        // LAddr app_heap;
        // Size  app_data_size;
        PAddr app_extra;
        Size  app_extra_size;
    };
    

public:
    Boot_Map bm;
    Load_Map lm;
};

__END_SYS

#endif
