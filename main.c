/**
 * @file : main.c
 * @brief : main
 * @details :
 * @author : Yicheng QIAN and Linxiang CONG
 * @version : beta 1.0.0
 * @date : 2023.3.26
 *
 */

#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/wait.h>

/**
 * @brief TEAMNB = 16, TEAMNB is the number of teams
*/
#define TEAMNB 16
/**
 * @brief PM = 2, PM is the number of points to be won at the end of each match
*/
#define PM 2

/**
 * @brief SHM_SIZE = TEAMNB*sizeof(Team)
*/
#define SHM_SIZE TEAMNB*sizeof(Team)


/**
 * @brief Team struct
 * @details Team struct contient name, Points won, et Points lost
 */
typedef struct team{
    char name[20]; /**< The team's name */
    int noteWin; /**< Points won by this team  */
    int noteLost; /**< Points lost by this team */
}Team;



/**
 * @brief mutexId, id of the mutex
*/
int mutexId;

/**
 * @brief sembuf sP, 
*/
static struct sembuf sP ={0,-1,0};

/**
 * @brief sembuf sV
*/
static struct sembuf sV = {0, 1,0};



/**
 * 
 * @brief Get the NoteWin of the team.
 *
 * @param team The team you specify.
 * @return The NoteWin of the team.
 */
int getNoteWin(Team team){
  return team.noteWin;
}


/**
 * 
 * @brief Get the NoteLost of the team.
 *
 * @param team The team you specify.
 * @return The NoteLost of the team.
 */
int getNoteLost(Team team){
  return team.noteLost;
}


/**
 * 
 * @brief Get the i-th team.
 *
 * @param teamList An array of Team types.
 * @param i The index you specify.
 * @return The team you want.
 */
Team getIElem(Team* teamList, int i){
  return teamList[i];
}


/**
 * 
 * @brief In the [start] to [end] subscript of the teamlist, find the first team that has played a [tour] round and won all of its matches..
 *
 * @param teamList An array of Team types.
 * @param start The start index.
 * @param end The end index.
 * @param tour Number of rounds of the competition. 
 * @return The index of the first team you want.
 */
int findT1(Team* teamList, int start, int end, int tour){
  for(int i = start; i <= end; i++){
    if(teamList[i].noteWin == tour){
      return i;
    }
  }
  return -1;
}


/**
 * 
 * @brief In the [start] to [end] subscript of the teamlist, find the second team that has played a [tour] round and won all of its matches.
 *
 * @param teamList An array of Team types.
 * @param start The start index.
 * @param end The end index.
 * @param tour Number of rounds of the competition. 
 * @return The index of the second team you want.
 */
int findT2(Team* teamList, int start, int end, int tour){
    for(int i = end; i >= start; i--){
      if(teamList[i].noteWin == tour){
        return i;
      }
    }
    return -1;
}


/**
 * 
 * @brief Traverse the array.
 *
 * @param teamList An array of Team types.
 * @param i The size of the array.
 */
void traverse(Team* teamList, int size){
  for(int i = 0; i < size; i++){
    Team team = getIElem(teamList, i);
    printf("team : %s\t\t noteWin = %d\t noteLost = %d\n",team.name, getNoteWin(team), getNoteLost(team));
  }
}


/**
 * 
 * @brief Get a array of char* types.
 * @details The return array is passed into the function and taken as the second argument
 *
 * @param size Total number of teams.
 * @param arr An empty array of char* to hold the results.
 * @param filepath The txt file from which the information needs to be extracted 
 */
void getTeamsName(int size, char (*arr)[16], char filePath[30]){
  FILE *f = fopen(filePath,"r");
  int i;
  for(i = 0; i < size; i++){
    fscanf(f, "#%[^#]#\n", arr[i]);
  }
  fclose(f);
}


/**
 * 
 * @brief block on a mutex
 *
 * @param mutexId the id of the mutex. 
 */
void P(int mutexId){ 
    semop(mutexId,&sP,1); 
}


/**
 * 
 * @brief unblock a mutex
 *
 * @param mutexId the id of the mutex. 
 */
void V(int mutexId) { 
    semop(mutexId,&sV,1); 
}


/**
 * 
 * @brief Simulation of two teams playing a match.
 *
 * @param t1 The index of the first team.
 * @param t2 The index of the second team.
 * @param pm Race point setting.
 * @param teamList An array of Team types.
 * @param tour Number of rounds of the competition. 
 * @param output the output file who stock results of each match 
 */
void match(int t1, int t2, int pm, Team* teamList,int tour, char* output){
  srand(time(NULL));
  // Initialize the actions of the attacker and the defender
  char* offensive[20] = {"essayer","botter"};
  char* defense[20] = {"tacler","Gatekept","GateKeepFail"};

  // sign = 0 : teamList[t1] is the attacking team
  // sign = 1 : teamList[t2] is the attacking team
  int signe = rand() % 2, offensiveIndex, defenseIndex;
  
  // Initialize the scores of both teams
  int potinT1=0,potinT2=0;


  P(mutexId);
  FILE *f = fopen(output,"a");

  printf("\033[43;31m\n\n%d tour\033[0m\n",tour);
  printf("\033[43;31m%-15s : %-12s\t<====>\t%-15s : %-12s\t%s => %d : %d\033[0m\n","OffensiveTeam","OffenAction","DefensiveTeam","DefenAction","_NOTE_",0,0);
  
  fprintf(f,"\n\n\n\n%d tour\n",tour);
  fprintf(f,"%-15s : %-12s\t<====>\t%-15s : %-12s\t%s => %d : %d\n","OffensiveTeam","OffAction","DefensiveTeam","DefenAction","_NOTE_",0,0);
  
  // Loop until one of the teams gets a match point
  while(potinT1 < pm && potinT2 < pm){
    offensiveIndex = rand() % 2;
    defenseIndex = rand() % 3;
    
    char* offensiveAction = offensive[offensiveIndex];
    char* defenseAction = defense[defenseIndex];
    
    if(strcmp(defenseAction, "tacler")==0 || strcmp(defenseAction, "GateKeepFail")==0){
      if(strcmp(defenseAction, "GateKeepFail") == 0){
        if(signe == 0){
          ++potinT1;
          printf("\n%s win 1 point!\n",getIElem(teamList,t1).name);
          fprintf(f,"\n%s win 1 point!\n",getIElem(teamList,t1).name);
        }
        else{
          ++potinT2;
          printf("\n%s win 1 point!\n",getIElem(teamList,t2).name);
          fprintf(f,"%s win 1 point!\n\n",getIElem(teamList,t2).name);
        }
      }
    }
    if(signe == 0){
      printf("%-15s : %-12s\t<====>\t%-15s : %-12s\t%s => %d : %d\n",getIElem(teamList,t1).name, offensiveAction, getIElem(teamList,t2).name,defenseAction,"_NOTE_",potinT1,potinT2);
      fprintf(f,"%-15s : %-12s\t<====>\t%-15s : %-12s\t%s => %d : %d\n",getIElem(teamList,t1).name, offensiveAction, getIElem(teamList,t2).name,defenseAction,"_NOTE_",potinT1,potinT2);
    }
    else{
      printf("%-15s : %-12s\t<====>\t%-15s : %-12s\t%s => %d : %d\n",getIElem(teamList,t2).name, offensiveAction, getIElem(teamList,t1).name,defenseAction,"_NOTE_",potinT2,potinT1);
      fprintf(f,"%-15s : %-12s\t<====>\t%-15s : %-12s\t%s => %d : %d\n",getIElem(teamList,t2).name, offensiveAction, getIElem(teamList,t1).name,defenseAction,"_NOTE_",potinT2,potinT1);
    }
    signe = rand() % 2;
    
    usleep(500000);
    
  }


  if(potinT1 == pm){
    int noteWin = teamList[t1].noteWin;
    teamList[t1].noteWin = noteWin + 1;

    int noteLost = teamList[t2].noteLost;
    teamList[t2].noteLost = noteLost + 1;

    printf("%s win %d match(es)\n", getIElem(teamList,t1).name,getNoteWin(getIElem(teamList,t1)));
    fprintf(f,"winner : %-15s\tloser : %-15s\n",getIElem(teamList,t1).name,getIElem(teamList,t2).name);
  }
  else{
    int noteWin = teamList[t2].noteWin;
    teamList[t2].noteWin = noteWin + 1;

    int noteLost = teamList[t1].noteLost;
    teamList[t1].noteLost = noteLost + 1;

    printf("%s win %d match(es)\n", getIElem(teamList,t2).name,getNoteWin(getIElem(teamList,t2)));
    fprintf(f,"winner : %-15s\tloser : %-15s\n",getIElem(teamList,t2).name,getIElem(teamList,t1).name);
  }
  

  fclose(f);

  V(mutexId);
  f = NULL;

}






int main(int argc, char *argv[]){
  if(argc < 4){
		exit(0);
	}

  int size = atoi(argv[1]);
	char nbE[size][16];

  getTeamsName(size,nbE ,argv[2]);

  int shmid;
  Team *teamList;

  // Create the shared memory segment with read and write permissions for all users
  key_t key = ftok(".", 1);
  if ((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0) {
      perror("shmget");
      exit(1);
  }

  // Attach the shared memory segment to our data space
  if ((teamList = shmat(shmid, NULL, 0)) == (Team *) -1) {
      perror("shmat");
      exit(1);
  }

  // Application of mutex
  key_t key2 = ftok(".",0);
  mutexId = semget(key2,1,IPC_CREAT|IPC_EXCL|0600);
  semctl(mutexId,0,SETVAL,1);

  // Initialization the array of Team types
  for (int i = 0; i < TEAMNB; i++) {
        strcpy(teamList[i].name, nbE[i]);
        teamList[i].noteWin = 0;
        teamList[i].noteLost = 0;
  }

  pid_t pid;
  
  int span;
  // tour
  for(int i = 1; i <= log(TEAMNB)/log(2.0); i++){
    span = pow(2,i) ;

    int j;
    for(j = 0; j < TEAMNB; j=j+span){
      int t1 = findT1(teamList, j, j+span-1,i-1);
      int t2 = findT2(teamList, j, j+span-1,i-1);

      switch(pid = fork()){
        case -1:
          perror("fork");
          exit(-1);
        case 0:
          
          match(t1,t2,PM,teamList,i,argv[3]);
          
          exit(0);
      }
    }
    for(int k = 0; k < j;k++){
      wait(NULL);
    }
  }

  traverse(teamList,16);

  // Removing a mutex
  semctl(mutexId,0,IPC_RMID,0);

  // Detach the shared memory segment
  if (shmdt(teamList) == -1) {
      perror("shmdt");
      exit(1);
  }


  return 0;
}




