/* spinlocks: code taken from xv6 */

#include "spinlock.h"
#include "arch.h"

void acquire(spinlock *lk)
{
    // disable interrupts to avoid deadlock.
    disable_interrupts();

    // On RISC-V, gcc compiles sync_lock_test_and_set(lk, 1) as (see `kernel.asm`):
    //   a5 = 1
    //   s1 = lk
    //   amoswap.w.aq a5, a5, (s1)
    // Instruction amoswap.w.aq rd, rs2, (rs1) atomically does:
    // 1. Load a word (32 bits) from memory in rd the value with address in rs1
    // 2. Store rs2 value in address in rs1
    // 3. The aq semantics ensure no memory operations following can be observed
    //    before this operations.
    while(__sync_lock_test_and_set(lk, 1) != 0)
        ;

    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that the critical section's memory
    // references happen strictly after the lock is acquired.
    // On RISC-V, this emits a fence instruction.
    __sync_synchronize();
}

void release(spinlock *lk)
{
    // Tell the C compiler and the CPU to not move loads or stores
    // past this point, to ensure that all the stores in the critical
    // section are visible to other CPUs before the lock is released,
    // and that loads in the critical section occur strictly before
    // the lock is released.
    // On RISC-V, this emits a fence instruction.
    __sync_synchronize();

    // Release the lock, equivalent to lk->locked = 0.
    // This code doesn't use a C assignment, since the C standard
    // implies that an assignment might be implemented with
    // multiple store instructions.
    // On RISC-V, gcc compiles sync_lock_release(lk) as:
    //   s1 = lk
    //   amoswap.w zero, zero, (s1)
    __sync_lock_release(lk);

    enable_interrupts();
}
