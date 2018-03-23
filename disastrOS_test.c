#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include "disastrOS.h"
#include "disastrOS_constants.h"

#define CICLES 5
#define NUMPRODUCER 2

void produce(int prod_id,int cons_id){
    for (int i = 0; i < CICLES; i++){
        //disastrOS_printStatus();
        disastrOS_semwait(prod_id);
        printf("PRODUCED\n");
        disastrOS_sempost(cons_id);
    }
}

void consume(int prod_id,int cons_id){
    for (int i = 0; i < CICLES; i++){
        //disastrOS_printStatus();
        disastrOS_semwait(cons_id);
        printf("CONSUMED\n");
        disastrOS_sempost(prod_id);
    }
}
// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n", disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n", disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(), type, mode);
  printf("fd=%d\n", fd);

  printf("Opening the semaphores...\n");

  int prod_id = disastrOS_semopen(1,NUMPRODUCER);
  
  int cons_id = disastrOS_semopen(2,0);

  printf("********WAITING SOME SECS BEFORE PRODUCING********\n");
  disastrOS_sleep(20);


  if (disastrOS_getpid() == 3) {
      printf("********PROCESS N 3 WILL PRODUCE AND PROCESS N 4 WILL CONSUME********\n");
      produce(prod_id, cons_id);
  }

  if (disastrOS_getpid() == 4){
      consume(prod_id, cons_id);
  }
  printf("PID: %d, terminating\n", disastrOS_getpid());


  /*for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);


    disastrOS_sleep((20-disastrOS_getpid())*5);
  }*/
  printf("********CLOSING SEMAPHORES********\n");
  disastrOS_semclose(prod_id);
  disastrOS_semclose(cons_id);
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  //int fd = disastrOS_semopen(1, 2);
  //int fill = disastrOS_semopen(2, 0);

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;

  for (int i=0; i<3; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);




    //printf("Sto chiudendo il semaforo #%d... \n",fd1);
    //int x = disastrOS_semclose(fd1);
    //printf("x=%d\n", x);


    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
   // disastrOS_printStatus();
    //printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	  // pid, retval, alive_children);
    --alive_children;
  }
  disastrOS_printStatus();
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
