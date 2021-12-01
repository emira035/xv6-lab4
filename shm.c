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
acquire(&(shm_table.lock));
int exists;
int i;

exists  = 0;

for(i=0; i <64;i++)
{
     page = &shm_table.shm_pages[i];
    if(page->id == id)
    {
      cprintf("WE FOUND A MATCH!\n");
      exists = 1;
      break;
    }
}

if(!exists)
{
  int emptypageindex=-1;

    for(i=0; i<64;i++)
    {
        if (shm_table.shm_pages[i].refcnt ==0)
        {
          emptypageindex = i;
          break;
        }  
    }

     
     //page  = &shm_table.shm_pages[emptypageindex];
     page->frame = kalloc();
     page->refcnt =1;    

     uint sz = PGROUNDUP(myproc()->sz);
    mappages(myproc()->pgdir,(char*)sz,PGSIZE,V2P(page->frame),PTE_W|PTE_U);
}





release(&(shm_table.lock));




return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!




return 0; //added to remove compiler warning -- you should decide what to return
}
