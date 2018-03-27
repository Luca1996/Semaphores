#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){

    /* - taking sem fd from syscall
       - retrieving SemDescriptor from process SemDescriptor list
       - returning an error if we do not find it
       - we remove it from process' SemDescriptor list
    */

    int fd = running->syscall_args[0];
    SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, fd);

    if (!sem_desc) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
        return;
    }

    List_detach(&running->sem_descriptors, (ListItem*)sem_desc);

   /* - taking semaphore from the descriptor
      - returning an error if we don't find it
    */

    Semaphore* sem = sem_desc->semaphore;

    if (!sem) {
        running->return_value = DSOS_ESEMAPHORENOTAVAIBLE;
        return;
    }

    /* - taking SemDescriptorPtr from sem_descriptor's list
       - returning an error if we don't find it
    */

    SemDescriptorPtr* sem_desc_ptr = (SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*) sem_desc->ptr);

    if (!sem_desc_ptr) {
        running->syscall_retvalue = DSOS_ESEMAPHOREDESC;
        return;
    }

    // if we don't have descriptors in the semaphore we remove it from the global list

    if (sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0) {
        List_detach(&semaphores_list,(ListItem*)sem);
        Semaphore_free(sem);
    }

    /* everything's fine so we free the resources and return from system call */

    SemDescriptor_free(sem_desc);
    SemDescriptorPtr_free(sem_desc_ptr);
    running->syscall_retvalue = 0;
    return;
}
