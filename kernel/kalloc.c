// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

int pageUseCount[PGROUNDUP(PHYSTOP) / PGSIZE + 1];

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;


void pUCcheck() {
  for(int i=0;i<PGROUNDUP(PHYSTOP) / PGSIZE + 1; ++i) {
    if(pageUseCount[i] != 0) { 
      printf("%d %d\n", i, pageUseCount[i]);
    } 
  }
  return;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  memset(pageUseCount, 0, sizeof(pageUseCount));
  freerange(end, (void*)PHYSTOP);
  pUCcheck();
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) { 
    uint64 PG = (uint64)p / PGSIZE;
    pageUseCount[PG] = 1;
    kfree(p);
    if(pageUseCount[PG] != 0) { 
      printf("Warning Warning Freerange\n");
    } 
  } 
  printf("FinPage %d\n", (uint64) p / PGSIZE);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP) {
    if(((uint64)pa % PGSIZE) != 0) 
      panic("kfree1");
    else if((char*)pa < end) {
      panic("kfree2");
    }
    else {
//      printf("GGGG %p\n", pa);
      panic("kfree3");
    }
  } 
  
  acquire(&kmem.lock);
  uint64 PG = (uint64)pa / PGSIZE;
//  printf("Kfree %p    %d\n", pa, pageUseCount[PG]);

  if(pageUseCount[PG] == 1) {
    pageUseCount[PG] = 0;
    memset(pa, 1, PGSIZE);
    r = (struct run*)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
  } 
  else if(pageUseCount[PG] < 1) {
//    printf("pageUseCount is now below 1 %d\n", pageUseCount[PG]);
    panic("pageUseCount is now below 1");

  }
  else {
 //   printf("PUC > 1 from kfree\n");
    pageUseCount[PG]--;
  }
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.

void *
kalloc(void)
{
//  printf("Kalloc\n");
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) { 
    uint64 PG = (uint64)r / PGSIZE;
    // if(pageUseCount[PG] != 0) { 
    //   pUCcheck();
    //   printf("Fault %d %d\n", PG, pageUseCount[PG]);
    //   panic("Something going wrong when kalloc a new page.");
    // } 
    pageUseCount[PG] = 1;
    kmem.freelist = r->next;
  } 
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  if((uint64)r / PGSIZE == 556904) {
//    printf("Warning Warning Important Page from kalloc %d\n", pageUseCount[556904]);
  }
  return (void*)r;
}

void increasePageUseCount(uint64 pg) { 
  acquire(&kmem.lock);  
  pageUseCount[pg]++;
  release(&kmem.lock);
} 

int getPageUseCount(uint64 pg) { 
  return pageUseCount[pg];
} 
