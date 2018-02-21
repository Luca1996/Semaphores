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
    /* I'm going to follow the 7 steps done while making disastrOS_open_resource.
       Ready, set, go! =)
    */
    // 1) Get info from PCB:
    printf("Getting info from PCB...\n");
    int id = running->syscall_args[0];
    int count = running->syscall_args[1];
    int open_mode=running->syscall_args[2];
    printf("Open_mode: %d \n", open_mode);

    Semaphore* sem = SemaphoreList_byId(&semaphores_list, id);
    printf("Information acquired!\n");
    printf("Sem's id is: %d\n", id);
    printf("Sem's count is: %d\n", count);

    /* 2) a. Check if the semaphore is already opened
          b. If not we add it to semaphores_list:
    */
    printf("Check if the sem is already opened...\n");
    //ho seguito gli step di open_resource.c, anche se non so se è necessario fare questa cosa anche nel caso dei semafori
    if (open_mode&DSOS_CREATE){
        // non entra mai qui!!!
        printf("Sono alla riga 34 di sem_open.c");
        if (sem) {
            printf("Sono alla riga 36 di sem_open.c");
            running->syscall_retvalue=DSOS_ERESOURCECREATE;
            return;
        }
        sem=Semaphore_alloc(id, count);
        List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);
        printf("Sem added to sems_list! \n");
    }


    printf("Check if all is ok...\n");

    // 3) Check if something gone wrong:
    if (! sem || sem->count!=count) {
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
    }
    printf("All is ok! \n");

    /* 4) a. Create the descriptor for the semaphore;
          b. Add it to process descriptors list;
          c. Assign to the semaphore just created a fd:
    */
    printf("Creating sem's descriptor...\n");
    printf("Adding it to descriptors list\n");
    printf("Allocating fd for the sem...\n");
    SemDescriptor* des = SemDescriptor_alloc(running->last_fd, sem, running);
    if (!des) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
        return;
    }
    printf("All done, fd allocated!\n");

    // 5) Increment the fd value for the next call:
    running->last_sem_fd++;
    SemDescriptorPtr* desptr = SemDescriptorPtr_alloc(des);
    List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) des);
    printf("Incremented the fd value for the next call! \n");

    // 6) Add a poiter to the newly created descriptor in the descriptor's pointer list:
    des->ptr = desptr;
    List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desptr);
    printf("Added a pointer to the newly created descriptor in poiter list! \n");

    // 7) Return the file descriptor to the process:
    running->syscall_retvalue = des->fd;
    return;

}
