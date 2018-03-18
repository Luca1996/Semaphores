#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){

    /* - taking sem id from syscall
       - taking SemDescriptor from process' SemDescriptor list
       - returning an error if we don't find it
    */

    int id = running->syscall_args[0];
    SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, id);

    if (!sem_desc) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
        return;
    }

    /* - taking Semaphore from the SemDescriptor
       - returning an error if we don't find it
    */

    Semaphore* sem = sem_desc->semaphore;

    if (!sem) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOTAVAIBLE;
        return;
    }

    /* - decrementing sem count
       - if sem count is <= 0:
            - we insert the running process in ready list
            - we take the SemDescriptor from the semaphore waiting list
            - we insert the SemDescriptorPtr of the previous SemDescriptor in the sem descriptor list
            - we remove the SemDescriptor's process from the waiting list and set it to Running
            - we set the calling process status to ready
    */

    SemDescriptorPtr* proc_desptr;
    (sem->count)++;

    if (sem->count <= 0) {
        //sem->count = 0;
        List_insert(&ready_list, ready_list.last, (ListItem*) running);
        proc_desptr = (SemDescriptorPtr*) List_detach(&sem->waiting_descriptors, (ListItem*) sem->waiting_descriptors.first);
        List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) proc_desptr);
        List_detach(&waiting_list, (ListItem*) proc_desptr->descriptor->pcb);
        running->status = Ready;
        running = proc_desptr->descriptor->pcb;

    }

    /* everything's fine so we return 0 */

    running->syscall_retvalue = 0;
    return;
}
