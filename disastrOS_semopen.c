#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"


void internal_semOpen(){
  /* I'm going to follow the 7 steps done while making disastrOS_open_resource:
      1) Get info from PCB
      2) a. Check if the semaphore is already opened
         b. If not we add it to semaphores_list
      3) Check if something gone wrong
      4) a. Create the descriptor for the semaphore;
         b. Add it to process descriptors list;
         c. Assign to the semaphore just created a fd;
      5) Increment the fd value for the next call
      6) Add a poiter to the newly created descriptor in the descriptor's
         pointer list
      7) Return the file descriptor to the process

      Ready, set, go! =)

  */
  // STEP 1:
  int id = running->syscall_args[0];
  int count = running->syscall_args[1];
  Semaphore* sem = SemaphoreList_byId(&semaphores_list, id);

  //STEP 2:
  if (!sem){
        sem = Semaphore_alloc(id, count);
        List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);
    }

  //STEP 3:
  if (! sem || sem->count!=count) {
     running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
     return;
  }

  //STEP 4:
  SemDescriptor* des = SemDescriptor_alloc(running->last_fd, sem, running);
  if (!des) {
     running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
     return;
  }

  //STEP 5:
  running->last_sem_fd++;
  SemDescriptorPtr* desptr = SemDescriptorPtr_alloc(des);
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) des);

  //STEP 6:
  des->ptr = desptr;
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desptr);

  //STEP 7:
  running->syscall_retvalue = des->fd;
  return;

}
