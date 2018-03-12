#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){

    /* - taking sem fd from syscall
       - retrieving SemDescriptor from process SemDescriptor list
       - returning an error if we do not find it
    */

    int fd = running->syscall_args[0];
    SemDescriptor* des = SemDescriptorList_byFd(&running->sem_descriptors, fd);

    if (!des) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
        return;
    }

    /* - taking SemDesciptorPtr from the previous SemDescriptor
       - returning an error if we don't find it
    */

    SemDescriptorPtr* descptr = des->ptr;

    if (!descptr) {
        running->syscall_retvalue = DSOS_ESEMAPHOREDESC;
        return;
    }

    /* - taking Semaphore from the SemDescriptor
       - returning an error if we don't find it
    */

    Semaphore* sem = des->semaphore;

    if (!sem) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOTAVAIBLE;
        return;
    }

    /* - decrementing Semaphore
       - checking sem count
       - if it is  < 0:
            - we remove the SemDescriptorPtr from the semaphore list
            - we insert the calling process in the semaphore waiting list
            - we insert the calling process in the processes waiting list
            - then we modify his status from running to waiting
            - we take the first ready process and we put this in running state
    */

    PCB* p;
    sem->count=(sem->count-1);

    if(sem->count < 0){
        List_detach(&sem->descriptors, (ListItem*) descptr);
        des = (SemDescriptor*) List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*) des);
        //sem->count=0;
        List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
        running->status = Waiting;
        p = (PCB*) List_detach(&ready_list, (ListItem*) ready_list.first);
        running = (PCB*)p;
    }

    /* everything's fine so we return 0 */
    running->syscall_retvalue=0;
    return;
}
