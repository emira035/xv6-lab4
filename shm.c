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


// Look through the shared memory table to find a matching segment id.
// If it doesnt exist,allocate a page for it and store this inside the table.
// using the physical address from page, create a mapping between the virtual and physical address using mappages.
// keep track of reference count 'refcnt' for page and set pointer to the virtual address using 'newsz'

int shm_open(int id, char **pointer) {

struct shm_page *page; 
struct proc* curproc = myproc();

acquire(&(shm_table.lock)); // acquire the shared memory lock because we will be working on the shm table
uint pageInd = 64;
int i;


//loop through all 64 pages to find the page with the matching id.
for(i=0; i <64;i++)
{
    page = &shm_table.shm_pages[i];

    if (page->refcnt == 0) // if the reference count is 0, means it has not been allocated.
      pageInd = i;      // use an unallocated page in case we don't find the id

    if(page->id == id)   // if we find a matching id
    {
      pageInd = i;      // immediately break so pageInd not written over by an id 0
      break;
    }
}

if (pageInd == 64) {    // unnecessary edge case
  release(&(shm_table.lock));
  return 0;
}

uint newsz = PGROUNDUP(curproc->sz); // va space where we can use to map page frame.

//case 2 :  shared memory segment is not found.
if (page->refcnt == 0) { 
  page->id = id;
  page->frame = kalloc(); //allocate one page and store its physical page.
  memset(page->frame, 0, PGSIZE); // sets the block of memory (page) to zero
  mappages(curproc->pgdir, (void*)newsz, PGSIZE, V2P(page->frame), PTE_W|PTE_U); // map the page
  //cprintf("%d %d\n", *pointer, newsz);
  *pointer = (char*)(newsz); //set pointer that points to virtual address
  curproc->sz = newsz + PGSIZE; //update sz because its va space grew
  page->refcnt = 1;  // set the reference count to 1 because its the first process to access page.


} 
//case 1: id matches to a shared memory segment
else { 
  mappages(curproc->pgdir, (void*)newsz, PGSIZE, V2P(page->frame), PTE_W|PTE_U);// map the page
  *pointer = (char*)(newsz); //set pointer that points to virtual address
  curproc->sz = newsz + PGSIZE; //update sz because its va space grew
  page->refcnt++; // increase reference count by 1 because it has been previously accessed
}


// we are done working with the shm table we can release the lock so someone else can work on the table.
release(&(shm_table.lock));




return 0; //added to remove compiler warning -- you should decide what to return
}



// look through the shm table and search for a page with the specified id
// if id is found, either decrement the reference count or clear the page.

int shm_close(int id) {
//you write this too!
uint i;
struct shm_page* page;

// acquire lock for the shm table
acquire(&(shm_table.lock));

//loop through all 64 pages to find the page with the specified id
for(i=0; i <64;i++)
{
    page = &shm_table.shm_pages[i];

    if(page->id == id) // we found the page with the specified id
    {
      if (page->refcnt > 1) { /// if its ref count is greater then 1 we just decrement it
        page->refcnt--;
      } else { // occurs when the ref count is at 1, we clear the page here
        memset(page, 0, sizeof(struct shm_page));
      }

      //release lock so someone else can work on shm table
      release(&(shm_table.lock));
      return 0; //added to remove compiler warning -- you should decide what to return
    }
}

release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}