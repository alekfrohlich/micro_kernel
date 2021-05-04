// EPOS RISC-V 32 CPU Mediator Implementation

#include <architecture/rv32/rv32_cpu.h>
#include <system.h>

extern "C" { void _exec(void *); }

__BEGIN_SYS

// Class attributes
unsigned int CPU::_cpu_clock;
unsigned int CPU::_bus_clock;

// Class methods
void CPU::Context::save() volatile //!P4: Unused, as of now
{
    ASM("       csrr     gp,  sstatus           \n"
        "       sw       gp, -120(sp)           \n"     // push sstatus
        "       auipc    gp, 0                  \n" 
        "       sw       gp, -116(sp)           \n"     // push pc
        "       sw       x1, -112(sp)           \n"     // push ra
        "       sw       x5, -108(sp)           \n"     // push x5-x31
        "       sw       x6, -104(sp)           \n"
        "       sw       x7, -100(sp)           \n"
        "       sw       x8,  -96(sp)           \n"
        "       sw       x9,  -92(sp)           \n"
        "       sw      x10,  -88(sp)           \n"
        "       sw      x11,  -84(sp)           \n"
        "       sw      x12,  -80(sp)           \n"
        "       sw      x13,  -76(sp)           \n"
        "       sw      x14,  -72(sp)           \n"
        "       sw      x15,  -68(sp)           \n"
        "       sw      x16,  -64(sp)           \n"
        "       sw      x17,  -60(sp)           \n"
        "       sw      x18,  -56(sp)           \n"
        "       sw      x19,  -52(sp)           \n"
        "       sw      x20,  -48(sp)           \n"
        "       sw      x21,  -44(sp)           \n"
        "       sw      x22,  -40(sp)           \n"
        "       sw      x23,  -36(sp)           \n"
        "       sw      x24,  -32(sp)           \n"
        "       sw      x25,  -28(sp)           \n"
        "       sw      x26,  -24(sp)           \n"
        "       sw      x27,  -20(sp)           \n"
        "       sw      x28,  -16(sp)           \n"
        "       sw      x29,  -12(sp)           \n"
        "       sw      x30,   -8(sp)           \n"
        "       sw      x31,   -4(sp)           \n");

    ASM("       addi     sp, sp, -120           \n"                     // complete the pushes above by adjusting the SP
        "       sw       sp, 0(%0)              \n" : : "r"(this));     // update the this pointer to match the context saved on the stack
}

// Context load does not verify if interrupts were previously enabled by the Context's constructor
// We are setting sstatus to SPP_U | SPIE, therefore, interrupts will be enabled only after sret
void CPU::Context::load() const volatile
{
    ASM("       mv      sp, %0                  \n"                     // load the stack pointer with the this pointer
        "       addi    sp, sp, %1              \n" : : "r"(this), "i"(sizeof(CPU::Context)));     // adjust the stack pointer to match the subsequent series of pops

    ASM("       lw       x1, -112(sp)           \n"     // pop ra
        "       lw       x5, -108(sp)           \n"     // pop x5-x31
        "       lw       x6, -104(sp)           \n"
        "       lw       x7, -100(sp)           \n"
        "       lw       x8,  -96(sp)           \n"
        "       lw       x9,  -92(sp)           \n"
        "       lw      x10,  -88(sp)           \n"
        "       lw      x11,  -84(sp)           \n"
        "       lw      x12,  -80(sp)           \n"
        "       lw      x13,  -76(sp)           \n"
        "       lw      x14,  -72(sp)           \n"
        "       lw      x15,  -68(sp)           \n"
        "       lw      x16,  -64(sp)           \n"
        "       lw      x17,  -60(sp)           \n"
        "       lw      x18,  -56(sp)           \n"
        "       lw      x19,  -52(sp)           \n"
        "       lw      x20,  -48(sp)           \n"
        "       lw      x21,  -44(sp)           \n"
        "       lw      x22,  -40(sp)           \n"
        "       lw      x23,  -36(sp)           \n"
        "       lw      x24,  -32(sp)           \n"
        "       lw      x25,  -28(sp)           \n"
        "       lw      x26,  -24(sp)           \n"
        "       lw      x27,  -20(sp)           \n"
        "       lw      x28,  -16(sp)           \n"
        "       lw      x29,  -12(sp)           \n"
        "       lw      x30,   -8(sp)           \n"
        "       lw      x31,   -4(sp)           \n"
        "       lw       gp, -120(sp)           \n"     // pop sstatus
        "       csrs    sstatus,   gp           \n"     // set sstatus for sret
        "       lw       gp, -116(sp)           \n"     // pop pc
        "       csrw     sepc,     gp           \n"     // move pc to sepc for sret
        "       lw      sp,  -124(sp)           \n"     // Load user stack pointer
        "       sret                            \n");
}

void CPU::switch_context(Context ** o, Context * n, unsigned int change_satp, unsigned int new_satp)
{   
    // Push the context into the stack and update "o"
    ASM("       sw       x1, -116(sp)           \n"     // push the return address as pc
        "       sw       x1, -112(sp)           \n"     // push ra
        "       sw       x5, -108(sp)           \n"     // push x5-x31
        "       sw       x6, -104(sp)           \n"
        "       sw       x7, -100(sp)           \n"
        "       sw       x8,  -96(sp)           \n"
        "       sw       x9,  -92(sp)           \n"
        "       sw      x10,  -88(sp)           \n"
        "       sw      x11,  -84(sp)           \n"
        "       sw      x12,  -80(sp)           \n"
        "       sw      x13,  -76(sp)           \n"
        "       sw      x14,  -72(sp)           \n"
        "       sw      x15,  -68(sp)           \n"
        "       sw      x16,  -64(sp)           \n"
        "       sw      x17,  -60(sp)           \n"
        "       sw      x18,  -56(sp)           \n"
        "       sw      x19,  -52(sp)           \n"
        "       sw      x20,  -48(sp)           \n"
        "       sw      x21,  -44(sp)           \n"
        "       sw      x22,  -40(sp)           \n"
        "       sw      x23,  -36(sp)           \n"
        "       sw      x24,  -32(sp)           \n"
        "       sw      x25,  -28(sp)           \n"
        "       sw      x26,  -24(sp)           \n"
        "       sw      x27,  -20(sp)           \n"
        "       sw      x28,  -16(sp)           \n"
        "       sw      x29,  -12(sp)           \n"
        "       sw      x30,   -8(sp)           \n"
        "       sw      x31,   -4(sp)           \n"
        "       li      x31,    1 << 8          \n"     // we are inside the kernel, and so we must return in S-mode
        "       csrs    sstatus, x31            \n"
        "       csrr    x31,  sstatus           \n"     // get sstatus
        "       sw      x31, -120(sp)           \n"     // push sstatus
        "       addi     sp,      sp,   -120    \n"     // complete the pushes above by adjusting the SP
        "       sw       sp,    0(a0)           \n");   // update Context * volatile * o
        
    //!P4: We should switch AS here
    ASM("       beq     a2,  x0, load_new_context      \n"
        "       csrw    satp, a3                       \n"
        "       sfence.vma                             \n");
    
    
    ASM("load_new_context:");
    // Set the stack pointer to "n" and pop the context from the stack
    ASM("       mv       sp,      a1            \n"     // get Context * volatile n into SP
        "       addi     sp,      sp,    120    \n"     // adjust stack pointer as part of the subsequent pops
        "       lw      x31, -116(sp)           \n"     // pop pc to a temporary
        "       csrw    sepc, x31               \n"
        "       lw       x1, -112(sp)           \n"     // pop ra
        "       lw       x5, -108(sp)           \n"     // pop x5-x31
        "       lw       x6, -104(sp)           \n"
        "       lw       x7, -100(sp)           \n"
        "       lw       x8,  -96(sp)           \n"
        "       lw       x9,  -92(sp)           \n"
        "       lw      x10,  -88(sp)           \n"
        "       lw      x11,  -84(sp)           \n"
        "       lw      x12,  -80(sp)           \n"
        "       lw      x13,  -76(sp)           \n"
        "       lw      x14,  -72(sp)           \n"
        "       lw      x15,  -68(sp)           \n"
        "       lw      x16,  -64(sp)           \n"
        "       lw      x17,  -60(sp)           \n"
        "       lw      x18,  -56(sp)           \n"
        "       lw      x19,  -52(sp)           \n"
        "       lw      x20,  -48(sp)           \n"
        "       lw      x21,  -44(sp)           \n"
        "       lw      x22,  -40(sp)           \n"
        "       lw      x23,  -36(sp)           \n"
        "       lw      x24,  -32(sp)           \n"
        "       lw      x25,  -28(sp)           \n"
        "       lw      x26,  -24(sp)           \n"
        "       lw      x27,  -20(sp)           \n"
        "       lw      x28,  -16(sp)           \n"
        "       lw      x29,  -12(sp)           \n"
        "       lw      x31, -120(sp)           \n"     // pop sstatus
        "       csrw     sstatus, x31           \n"
        "       lw      x30,   -8(sp)           \n"
        "       lw      x31,   -4(sp)           \n"
        "       sret                            \n");
}

//!TODO: write message to a0
void CPU::syscall(void * message){
    ASM("  ecall  \n");
}

void CPU::syscalled() {
    ASM(
        " addi sp, sp, -12   \n"
        " sw   a0, 4(sp)    \n"
        " sw   ra, 8(sp)    \n"
        " call _exec        \n"
        " lw   a0, 4(sp)    \n"
        " lw   ra, 8(sp)    \n"
        " addi sp, sp,  12   \n"
    );
}


__END_SYS
