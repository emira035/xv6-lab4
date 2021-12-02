#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

struct shm_page *page;
struct proc* curproc = myproc();
acquire(&(shm_table.lock));
uint pageInd = 64;
int i;

for(i=0; i <64;i++)
{
    page = &shm_table.shm_pages[i];
    if (page->refcnt == 0)
      pageInd = i;      // use an unallocated page in case we don't find the id
    if(page->id == id)
    {
      pageInd = i;      // immediately break so pageInd not written over by an id 0
      break;
    }
}

if (pageInd == 64) {    // unnecessary edge case
  release(&(shm_table.lock));
  return 0;
}

uint newsz = PGROUNDUP(curproc->sz);
if (page->refcnt == 0) {
  page->id = id;
  page->frame = kalloc();
  memset(page->frame, 0, PGSIZE);
  mappages(curproc->pgdir, (void*)newsz, PGSIZE, V2P(page->frame), PTE_W|PTE_U);
  //cprintf("%d %d\n", *pointer, newsz);
  *pointer = (char*)(newsz); 
  curproc->sz = newsz + PGSIZE;
  page->refcnt = 1;
} else {
  mappages(curproc->pgdir, (void*)newsz, PGSIZE, V2P(page->frame), PTE_W|PTE_U);
  *pointer = (char*)(newsz);
  curproc->sz = newsz + PGSIZE;
  page->refcnt++;
}



release(&(shm_table.lock));




return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
uint i;
struct shm_page* page;

acquire(&(shm_table.lock));
for(i=0; i <64;i++)
{
    page = &shm_table.shm_pages[i];
    if(page->id == id)
    {
      if (page->refcnt > 1) {
        page->refcnt--;
      } else {
        memset(page, 0, sizeof(struct shm_page));
      }
      release(&(shm_table.lock));
      return 0; //added to remove compiler warning -- you should decide what to return
    }
}

release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}

