#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
    // we first take the fd of the semaphore
    // then we look for the semaphore

    // prendere l'id non il descrittore
    int fd = running->syscall_args[0];
    Semaphore* sem = SemaphoreList_byId(&semaphores_list, fd);//aggiunto questo,in questo modo mi prendo direttamente il semaforo
    // da modificare il resto
    
    // checking semaphore descriptor, if not found we raise an error

    if (!sem_desc) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOFD;
        return;
    }

    //removing semdescriptor from the list
    sem_desc = (SemDescriptor*) List_detach(&running->sem_descriptors,(ListItem*)sem_desc);

    // checking sem descriptor

    if (!sem_desc) {
        running->syscall_retvalue = DSOS_ESEMAPHORECLOSE;
        return;
    }

    // taking the semaphore from the descriptor

    Semaphore* sem = sem_desc->semaphore;

    // now we can remove the descriptorPtr

    SemDescriptorPtr* sem_desc_ptr = (SemDescriptorPtr*) List_detach(&sem->descriptors,(ListItem*)sem_desc->ptr);
    
    // checking pointer

    if (!sem_desc_ptr) {
        running->syscall_retvalue = DSOS_ESEMAPHOREDESC;
        return;
    }

    SemDescriptor_free(sem_desc);
    SemDescriptorPtr_free(sem_desc_ptr);
    running->syscall_retvalue = 0;
    return;
}