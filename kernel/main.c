#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if(cpuid() == 0){
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();         // physical page allocator
    printf("Alive after kinit                            \n");
    kvminit();       // create kernel page table
    printf("Alive after kvminit                            \n");
    kvminithart();   // turn on paging
    printf("Alive after kvminithart                            \n");
    procinit();      // process table
    printf("Alive after procinit                            \n");
    trapinit();      // trap vectors
    printf("Alive after trapinit                            \n");
    trapinithart();  // install kernel trap vector
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    iinit();         // inode table
    fileinit();      // file table
    virtio_disk_init(); // emulated hard disk
    userinit();      // first user process
    __sync_synchronize();
    printf("Alive afterALL                                     \n");
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
    printf("Alive after else all\n");
  }

  scheduler();        
}
