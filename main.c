// Semaphores SOI
// Bartlomiej Kulik
// 14.04.2018

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0

// VARIABLES
int N; // number of bolids
int K; // number of services
int P; // priority vaule
int L; // number of visits in services

// shared memory below
int *inService;
int *readyToEntry;
int *readyToEscape;

int *freeServiceTrack;
// end of shared memory
// semaphores
int semId;
struct sembuf semOpArg = {0, 0, 0};
enum { acces, queueEntry, queueExit};
// key to semaphores and shared memory
key_t key;

// PROTOTYPES
int areValid(int argc, char **argv);
void startWork();
int runTasks();
void prepareSharedMemory();
void endWork();
void P(int semaphore);
void V(int semaphore);

// MAIN FUNCTION
int main(int argc, char **argv)
{
  // set N,K,P,L
	if(!areValid(argc,argv))
  {
    return 1;
  }

	startWork();
	if(runTasks())
  {
    return 0;
  }
  endWork();

  return 0;
}

void startWork()
{
  int shmId;
  // MEMORY
  shmId = shmget(key, sizeof(int), 0600 | IPC_CREAT);
  inService = (int *)shmat(shmId, NULL, 0);
  *inService = 0;
  shmdt(inService);

  shmId = shmget(key + 1, sizeof(int), 0600 | IPC_CREAT);
  readyToEntry = (int *)shmat(shmId, NULL, 0);
  *readyToEntry = 0;
  shmdt(readyToEntry);

  shmId = shmget(key + 2, sizeof(int), 0600 | IPC_CREAT);
  readyToEscape = (int *)shmat(shmId, NULL, 0);
  *readyToEscape = 0;
  shmdt(readyToEscape);

  shmId = shmget(key + 3, sizeof(int), 0600 | IPC_CREAT);
  freeServiceTrack = (int *)shmat(shmId, NULL, 0);
  *freeServiceTrack = TRUE;
  shmdt(freeServiceTrack);
  // SEMAPHORES
  semId = semget(key, 3, 0600 | IPC_CREAT);
  semctl(semId, acces, SETVAL, 1);
  semctl(semId, queueExit, SETVAL, 0);
  semctl(semId, queueEntry, SETVAL, 0);

}

void prepareSharedMemory()
{
  int shmId;

  shmId = shmget(key, sizeof(int), 0600 | IPC_CREAT);
  inService = (int *)shmat(shmId, NULL, 0);

  shmId = shmget(key + 1, sizeof(int), 0600 | IPC_CREAT);
  readyToEntry = (int *)shmat(shmId, NULL, 0);

  shmId = shmget(key + 2, sizeof(int), 0600 | IPC_CREAT);
  readyToEscape = (int *)shmat(shmId, NULL, 0);

  shmId = shmget(key + 3, sizeof(int), 0600 | IPC_CREAT);
  freeServiceTrack = (int *)shmat(shmId, NULL, 0);

}

int runTasks()
{
  int pid;
  int i;
  for(i = 0; i < N; ++i)
  {
    pid = fork();
    if (pid == 0) // child
    {
      prepareSharedMemory();
      // go!
      bolid();
      // finish!
      return 1;
    }
    else
    {
      return 0;
    }
  }
}

void endWork()
{
    int processes;
    for(processes = N; processes > 0; --processes) // one pass
    {
      wait(NULL);
    }

    semctl(semId, 0, IPC_RMID);

    shmctl(inService, IPC_RMID, NULL);
    shmctl(readyToEntry, IPC_RMID, NULL);
    shmctl(readyToEscape, IPC_RMID, NULL);
    shmctl(freeServiceTrack, IPC_RMID, NULL);
}

void P(int semaphore)
{
  semOpArg.sem_num = semaphore;
  semOpArg.sem_op = -1;
  semop(semId, &semOpArg, 1);
}

void V(int semaphore)
{
  semOpArg.sem_num = semaphore;
  semOpArg.sem_op = 1;
  semop(semId, &semOpArg, 1);
}

int areValid(int argc, char **argv)
{
    if(argc!=5)
    {
      return 0;
    }
    N = atoi(argv[1]);
	  K = atoi(argv[2]);
    P = atoi(argv[3]);
    L = atoi(argv[4]);
    if(N <= 0 || K <= 0 || P <= 0 || L <= 0)
    {
      return 0;
    }
    // key set
    key_t key = ftok(argv[0], 1);
		return 1;
}
