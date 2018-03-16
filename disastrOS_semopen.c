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


    // getting info from syscall

    int id = running->syscall_args[0];
    int count = running->syscall_args[1];

    // using id to take semaphore

    Semaphore* sem = SemaphoreList_byId(&semaphores_list, id);
    
    // if count <= 0 we don't want to open sem

    if (count <= 0){
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
    }

    if (sem && running->sem_descriptors.size != 0 ) {
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
    }
    // if sem not open yet we alloc it

    if (!sem) {
        sem=Semaphore_alloc(id, count);
        List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);
    }

    /* - allocating SemDescriptor
       - raising an error if it fails
       - inserting it into process' sem_descriptors list
     */

    SemDescriptor* des = SemDescriptor_alloc(running->last_sem_fd, sem, running);

    if (!des) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
        return;
    }

    List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) des);

    // incrementing the fd value for the next call

    running->last_sem_fd++;

    /* - allocating SemDescriptorPtr
       - and raising an error if it fails
       - setting SemDescriptor's pointer to SemDescriptorPtr
       - inserting SemDescriptorPtr to sem's descriptors list
    */

    SemDescriptorPtr* desptr = SemDescriptorPtr_alloc(des);

    if (!desptr) {
        running->syscall_retvalue = DSOS_ESEMAPHOREDESC;
        return;
    }

    des->ptr = desptr;
    List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desptr);

    /* - everything's fine
       - we return from system call
    */

    running->syscall_retvalue = des->fd;
    return;

}
