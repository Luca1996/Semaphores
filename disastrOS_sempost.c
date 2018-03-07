#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
    // taking sem id
    int id = running->syscall_args[0];

    SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors,id);

    // error check

    if (!sem_desc) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
        return;
    }

    // taking sem to take his value

    Semaphore* sem = sem_desc->semaphore;
    
    SemDescriptor* proc_des;
    (sem->count)++;
    // checking value
    if (sem->count <= 0) {
        //sem->count = 0;
        // removing first one from the waiting list

        List_insert(&ready_list,ready_list.last,(ListItem*) running);

        proc_des = (SemDescriptor*)List_detach(&sem->waiting_descriptors,(ListItem*)sem->waiting_descriptors.first);
        List_insert(&sem->descriptors,sem->descriptors.last,(ListItem*) proc_des->ptr);
        List_detach(&waiting_list,(ListItem*) proc_des->pcb);
        running->status = Ready;
        running = proc_des->pcb;
        
    }
    // incrementing count
    

    running->syscall_retvalue = 0;
    return;
}
