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
    
    // checking value
    if (sem->count <= 0) {
        sem->count = 0;
        // removing first one from the waiting list
        List_detach(&waiting_list,(ListItem*)waiting_list.first);
    }
    // incrementing count
    sem->count++;

    // then we insert the process in waiting list
    List_insert(&waiting_list,waiting_list.last,(ListItem*)running);

    // we set process status on ready
    running->status = Ready;

    running->syscall_retvalue = 0;
    return;
}
