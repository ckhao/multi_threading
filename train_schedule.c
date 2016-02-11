/*
* CSC360 Assignment 2
* Multiple Threads  Scheduling  	
* 
* @Chenkai Hao V00819367
* June 18, 2015
*
*/


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h> 

#define MAX 100
#define TRUE 0;
#define FALSE -1;




int trackStatus = 1;//main track statues: availble = 1; unavailable = 0
int currentTid;
int trainNum = 0;//
long currentID;//the current train that is on the main track
int numOfWaitingTrains = 0; //current number of trains

//int counter; //NO IDEA HOW TO USE
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;//for function request()
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;//for track
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;//for last direction
pthread_cond_t dispatch = PTHREAD_COND_INITIALIZER;
//read the input file and format the train information
typedef struct
{
	long num;
	char dir;	//direction
	int loadT;	//loading time
	int crossT;	//crossing time
	
	int priority;
	char state;
}train;

train infoArray[MAX];//array that all the trains listed in input order



int waitlist[MAX];
int next;
int temp;

int lastDirection = -1;

int findNext(){
	
	int i;

	if(numOfWaitingTrains != 0){
		next = waitlist[0];
		for(i=0; i<numOfWaitingTrains; i++){
			//multiple loaded trains, the one with high priority crosses
			if(infoArray[waitlist[i]].priority > infoArray[next].priority){
				next = waitlist[i];
				temp = waitlist[0];
				waitlist[i] = temp;
				waitlist[0] = next;
			}else if(infoArray[waitlist[i]].priority == infoArray[next].priority){	//if two loaded trains have the same priority 
				//if same direction
				if(infoArray[waitlist[i]].dir == infoArray[next].dir){
					//who finishes loading first get on first
					if( infoArray[waitlist[i]].loadT < infoArray[next].loadT){
						next = waitlist[i];
						temp = waitlist[0];
						waitlist[i] = temp;
						waitlist[0] = next;
				
					//same loading time
					}else if(infoArray[waitlist[i]].loadT == infoArray[next].loadT){
						if(infoArray[waitlist[i]].num < infoArray[next].num){
							next = waitlist[i];
							temp = waitlist[0];
							waitlist[i] = temp;
							waitlist[0] = next;
						}
					}	
				}else if(infoArray[waitlist[i]].dir != infoArray[next].dir){//different direction
					//opposite of the last train go
					//printf("the last direction is %c\n", lastDirection);
					if((lastDirection == 1 || lastDirection == -1) && (infoArray[waitlist[i]].dir == 'W' || infoArray[waitlist[i]].dir == 'w')){
						next = waitlist[i];
						temp = waitlist[0];
						waitlist[i] = temp;
						waitlist[0] = next;
					
					//if no train has been crossed
					}else if(lastDirection == 0 && (infoArray[waitlist[i]].dir == 'e' || infoArray[waitlist[i]].dir == 'E')){
						next = waitlist[i];
						temp = waitlist[0];
						waitlist[i] = temp;
						waitlist[0] = next;
					}
				}
			
			}
		}
	}

	
	
	return TRUE;
}

//function to put a train into waitlist 
void request_track(long trainID){
	pthread_mutex_lock(&mutex1);
	//if the main track is available and no train is waiting
	if(trackStatus == 1 && numOfWaitingTrains == 0){
		pthread_mutex_lock(&mutex2);
		trackStatus = 0;//one train get on the main track
		pthread_mutex_unlock(&mutex2);
		pthread_mutex_unlock(&mutex1);
		return;
	}
	
	pthread_mutex_lock(&mutex2);
	waitlist[numOfWaitingTrains] = trainID;//add this train to waitlist

	numOfWaitingTrains++;

	if(findNext() != 0){
		printf("ERROR in finding next train\n");
	}
	pthread_mutex_unlock(&mutex2);
	
	//if the main track is available and there is some trains waiting
	if(trackStatus == 1 && numOfWaitingTrains != 0){
		pthread_mutex_lock(&mutex2);
		trackStatus = 0;
		pthread_mutex_unlock(&mutex2);
	}
	
	//when the track is unavailble(0), or the current train is not the first in the
	while(trackStatus == 0 || trainID != waitlist[0]){
		//printf("train #%ld is waiting for train #%d to cross\n", trainID, currentTid);
		pthread_cond_wait(&dispatch,&mutex1);
	}
	
	pthread_mutex_lock(&mutex3);
	if(infoArray[trainID].dir == 'E' || infoArray[trainID].dir == 'e'){
		//p = 1;
		lastDirection = 1;
	}else{
		lastDirection = 0;
	}
	pthread_mutex_unlock(&mutex3);

	//delete the next train in the waitlist
	pthread_mutex_lock(&mutex2);

	waitlist[0] = waitlist[numOfWaitingTrains-1];
	numOfWaitingTrains--;

	if(findNext() != 0){
		printf("ERROR in finding next train\n");
	}
	pthread_mutex_unlock(&mutex2);
	pthread_mutex_lock(&mutex2);
	//unlock mutex1 if the main track is available
	if(trackStatus == 1){
		pthread_mutex_unlock(&mutex1);
	}
	
	pthread_mutex_unlock(&mutex2);

}

//release service
void release_service(){
    pthread_mutex_lock(&mutex2);
    //set track to available
    trackStatus = 1;
    pthread_mutex_unlock(&mutex2);
    //broadcast the signal
    pthread_cond_broadcast(&dispatch);
}

void *threadControl(void *threadptr){
	long tid;
	tid = (long)threadptr;
	infoArray[tid].num = tid;
	double timeToLoad = (double)infoArray[tid].loadT/10;
	double timeToCross = (double)infoArray[tid].crossT/10;
	char* direction;
	if(infoArray[tid].dir == 'e'||infoArray[tid].dir == 'E'){
		direction = "East";
	}else{
		direction = "West";
	}


	if(infoArray[tid].dir == 'E' || infoArray[tid].dir == 'W'){
		//p = 1;
		infoArray[tid].priority = 1;
	}else{
		infoArray[tid].priority = 0;
	}
	
	//loading section
	printf("train %ld is loading for %f seconds.\n", tid, timeToLoad);
	//infoArray[tid].state = 'L'; //Change the state to L(loading)
	usleep(timeToLoad*1000000);
	//infoArray[tid].state = 'R'; //Change the state to R(ready to go)
	printf("train %ld finished loading.\n", tid);
	
	//crossing section
	request_track(tid);
	pthread_mutex_lock(&mutex3);
	if(infoArray[tid].dir == 'E' || infoArray[tid].dir == 'e'){
		lastDirection = 1;
	}
	if(infoArray[tid].dir == 'W' || infoArray[tid].dir== 'w'){
		lastDirection = 0;
	}

	pthread_mutex_unlock(&mutex3);
	
	//if the main track is availble (trackStatus=1) and no train is waiting: set trackStatus = 0
	currentTid = tid;
	if(trackStatus == 1){
		pthread_mutex_lock(&mutex2);
		trackStatus = 0;
		pthread_mutex_unlock(&mutex2);
	}	
	
	//pthread_mutex_lock(&mutex2);
	printf("train #%ld is ON the main track for %f seconds, heading %s\n", tid, timeToCross, direction);
	//infoArray[tid].state = 'G';
	usleep(timeToCross*1000000);
	printf("train #%ld is OFF the main track\n", tid);
	//pthread_mutex_unlock(&mutex2);	
	
	release_service();


	pthread_exit(NULL);
}


//function main() provides the scheduler for trains to get on the main track
int main( int argc, char **argv){

	int k;
	//initialize the array
	for(k=0; k<MAX; k++){
		waitlist[k] = 999;//999 means there is nothing
	}
	
	//init two counters
	int i,j;
	int quantity;
	
	int lines = 0;	
	
	//an array of typedef Info
	char line[MAX];
	//1. open an input file
	if(argc != 3){
		printf("please provide an proper input file and the number of trains\n");
	}else{
		FILE *file = fopen(argv[1], "r");
		
		if(file == 0){
			printf("Could not open the file\n");
		}else{
			quantity = atoi(argv[2]);
			//2. read the input file
			int x;
			

			while ( (fgets(line, sizeof(line), file)) != NULL){
				printf("%s",line);
				//3. parse the information to struct
				infoArray[lines].dir=*(strtok(line, ":,")); //direction
				infoArray[lines].loadT=atoi(strtok(NULL, ":,")); //loading time
				infoArray[lines].crossT=atoi(strtok(NULL, ":,")); //crossing timeckhaqo
				
				lines++;
			}
			printf("Total number of trains: %d\n", lines);
			
			//numberOfTrains = lines;


			fclose(file);	
		}
	}

	/********************NOW we are going to create threads!*********************/
	pthread_t threads[lines];

	//train train[lines];
	
	
	//trainPtr tp = &infoArray[0];

	int rc;
	long t;
	for(t=0;t<quantity;t++){
	   //printf("In main: creating thread %ld\n", t);
	   rc = pthread_create(&threads[t], NULL, threadControl, (void *) t);
	   //infoArray[t].num = t;
	   //tp++;
	   if (rc){
	       printf("ERROR; return code from pthread_create() is %d\n", rc);
	       exit(-1);
	   }
	}
	
	/*show things in the waitlist
	usleep(10000); 
	for(k=0; k<lines; k++){
		printf("%d ", waitlist[k]);
	}	
	*/

	for(t=0; t<lines;t++){
		pthread_join(threads[t],NULL);
	}	

	
	//pthread_attr_t attr[lines];




}
