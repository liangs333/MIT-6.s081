// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"


#define BUCKETSIZ (NBUF / 2)
#define BUCKETNUM (13)
// struct {
//   struct spinlock lock;
//   struct buf buf[NBUF];
//   // Linked list of all buffers, through prev/next.
//   // Sorted by how recently the buffer was used.
//   // head.next is most recent, head.prev is least.
//   struct buf head;
// } bcache;

struct hashBucket {
  struct spinlock lock;
  struct buf buf[BUCKETSIZ];
} bucket[BUCKETNUM];

int getHash(uint64 num) { 
  return num % BUCKETNUM;
} 

void
binit(void)
{

  struct buf *b;
  for(int nBucket = 0; nBucket < BUCKETNUM; ++nBucket) { 
    initlock(&bucket[nBucket].lock, "bcache");
    for(int i = 0; i < BUCKETSIZ; ++i) { 
      b = &bucket[nBucket].buf[i];
      b -> lastim = 0;
      b -> bucketNumber = i;
      initsleeplock(&b -> lock, "buffer");
    } 
  } 
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int hashVal = getHash(blockno);
  acquire(&bucket[hashVal].lock);
  for(int i = 0; i < BUCKETSIZ; ++i) {
    b = &bucket[hashVal].buf[i];
    if(b -> dev == dev && b -> blockno == blockno) {
      b -> refcnt++;
      b -> lastim = ticks;
      release(&bucket[hashVal].lock);
      acquiresleep(&b -> lock);
      return b;
    }
  }


  // acquire(&bcache.lock);
  // // Is the block already cached?
  // for(b = bcache.head.next; b != &bcache.head; b = b->next){
  //   if(b->dev == dev && b->blockno == blockno){
  //     b->refcnt++;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }

  uint64 mitim = 0x3f3f3f3f3f3f3f3f;
  struct buf* chosenOne = 0;
  for(int i = 0; i < BUCKETSIZ; ++i) {
    b = &bucket[hashVal].buf[i];
    if(b -> refcnt == 0 && b -> lastim < mitim) {
      mitim = b -> refcnt;
      chosenOne = b;
    }
  }


  b = chosenOne;

  if(mitim != b -> lastim) {
    panic("GG in bget");
  }

  b->dev = dev;
  b->blockno = blockno;
  b->valid = 0;
  b->refcnt = 1;
  b->lastim = ticks;
  release(&bucket[hashVal].lock);
  acquiresleep(&b->lock);
  return b;

  // // Not cached.
  // // Recycle the least recently used (LRU) unused buffer.
  // for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
  //   if(b->refcnt == 0) {
  //     b->dev = dev;
  //     b->blockno = blockno;
  //     b->valid = 0;
  //     b->refcnt = 1;
  //     release(&bcache.lock);
  //     acquiresleep(&b->lock);
  //     return b;
  //   }
  // }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;
  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bucket[b->bucketNumber].lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b -> lastim = 0;
    //感觉这里简单修改一下时间chuo2就行了
  }
  
  release(&bucket[b->bucketNumber].lock);
}

void
bpin(struct buf *b) {
  acquire(&bucket[b->bucketNumber].lock);
  b->refcnt++;
  release(&bucket[b->bucketNumber].lock);
}

void
bunpin(struct buf *b) {
  acquire(&bucket[b->bucketNumber].lock);
  b->refcnt--;
  release(&bucket[b->bucketNumber].lock);
}


