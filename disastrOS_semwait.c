#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){

    // 1) Like in the disastrOS_close_resource we need to find the semephore id which we need to put in wait:
    int id = running->syscall_args[0];

    SemDescriptor* des = SemDescriptorList_byFd(&running->sem_descriptors, id);

    // 2) If the id is not in the process we need to raise an error code:
    if (!des) {
        running->syscall_retvalue = DSOS_ESEMAPHORENOTAVAIBLE;
        return;
    }

    // 3) We need to check the value of the count, if count <= 0 we need to put the sem in waiting descriptors list:

    Semaphore* sem = des->semaphore;
    PCB* p;
    printf("Il count del semaforo prima della sottrazione e': %d\n",(sem->count));
    if(sem->count <= 0){
        //qui non entro mai
        printf("sono nella wait, punto 3\n");
        // cambiato da running a des. OK
        des = (SemDescriptor*) List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*) des);
        disastrOS_printStatus();

        //qui devo fare la detach su ready_list per prendere il primo processi pronto della lista
        p = (PCB*) List_detach(&ready_list, (ListItem*) ready_list.first);
        //poi devo fare l'insert del pcb preso qui sopra e metterlo in running
        running=(PCB*)p;


    }

    // 4) Now we can put in wait the sem by decreasing it's count value:
    printf("sono nella wait, punto 4\n");
    sem->count=(sem->count-1);
    printf("Il count del semaforo dopo la sottrazione e': %d",(sem->count));

    // 5) a. With the updated value of count, we need to know if our process has to wait:
    if(sem->count < 0){
        //qui non entro mai
         printf("sono nella wait, punto 5\n");
        sem->count=0;

        // b. The if is TRUE, so the process need to wait;
        // c. Let's remove it from running list and put it to waiting list:
        List_detach(&ready_list, (ListItem*) running);
        // da running a descriptor?? per ora metto running

        List_insert(&waiting_list, waiting_list.last, (ListItem*) running);

        // d. Let's update his status from running to waiting:
        running->status= Waiting;
    }

    // 6) Now we can return:
    running->syscall_retvalue=0;
    return;
}
