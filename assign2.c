/*
 * assign2.c
 *
 * Name: Daniel Truong
 * Student Number: V00795971
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include "train.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int allowedId = -1;

struct node{
	int id;
	struct node *ptr;
}*frontEast,*frontWest,*westTemp,*eastTemp, *temp, node;

int conCount = 0;
int westCount = 0;
int eastCount = 0;

/*create array id for all trains and set condit var to the train id that can leave. main will check through all the trains in the station.*/

/*
 * If you uncomment the following line, some debugging
 * output will be produced.
 *
 * Be sure to comment this line out again before you submit 
 */

/* #define DEBUG	1 */

void ArriveBridge (TrainInfo *train);
void CrossBridge (TrainInfo *train);
void LeaveBridge (TrainInfo *train);

/*
 * This function is started for each thread created by the
 * main thread.  Each thread is given a TrainInfo structure
 * that specifies information about the train the individual 
 * thread is supposed to simulate.
 */
void * Train ( void *arguments )
{
	TrainInfo	*train = (TrainInfo *)arguments;

	/* Sleep to simulate different arrival times */
	usleep (train->length*SLEEP_MULTIPLE);

	ArriveBridge (train);
	CrossBridge  (train);
	LeaveBridge  (train); 

	/* I decided that the paramter structure would be malloc'd 
	 * in the main thread, but the individual threads are responsible
	 * for freeing the memory.
	 *
	 * This way I didn't have to keep an array of parameter pointers
	 * in the main thread.
	 */
	free (train);
	return NULL;
}

/*
 * You will need to add code to this function to ensure that
 * the trains cross the bridge in the correct order.
 */
void ArriveBridge ( TrainInfo *train )
{
	printf ("Train %2d arrives going %s\n", train->trainId, 
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	pthread_mutex_lock(&mutex);
	int triggered = 0;
	struct node *current = (struct node *)malloc(1*sizeof(struct node));
	current->id = train->trainId;
	current->ptr = NULL;
	if(train->direction == 1){
		if(westCount == 0) frontWest = current;
		else westTemp->ptr = current;
		westCount++;
		westTemp = current;
	} else {
		if(eastCount == 0) frontEast = current;
		else eastTemp->ptr = current;
		eastCount++;
		eastTemp = current;
	}
	if(allowedId == -1){
		allowedId = train->trainId;
		if(train->direction == 2){
			triggered = 1;
			conCount++;
		}
	}
	pthread_mutex_unlock(&mutex);

	pthread_mutex_lock(&mutex);
	while(allowedId != train->trainId) pthread_cond_wait(&cond,&mutex);
	if(train->direction == 2 && conCount != 2 && triggered == 0) conCount++;
	pthread_mutex_unlock(&mutex);
}

/*
 * Simulate crossing the bridge.  You shouldn't have to change this
 * function.
 */
void CrossBridge ( TrainInfo *train )
{
	printf ("Train %2d is ON the bridge (%s)\n", train->trainId,
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
	
	/* 
	 * This sleep statement simulates the time it takes to 
	 * cross the bridge.  Longer trains take more time.
	 */
	usleep (train->length*SLEEP_MULTIPLE);

	printf ("Train %2d is OFF the bridge(%s)\n", train->trainId, 
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
}

/*
 * Add code here to make the bridge available to waiting
 * trains...
 */
void LeaveBridge ( TrainInfo *train )
{
	pthread_mutex_lock(&mutex);
  	if(train->direction == 1){
  		temp = frontWest;
  		frontWest = frontWest->ptr;
  		westCount--;
  		conCount = 0;
  	} else {
  		temp = frontEast;
  		frontEast = frontEast->ptr;
  		eastCount--;
  		if(westCount == 0) conCount--;
  	}
  	free(temp);

  	if(eastCount == 0 && westCount == 0) allowedId = 0;
  	else{
  		if((eastCount == 0 || conCount >= 2) && westCount > 0){
  			allowedId = frontWest->id;
  			conCount = 0; 
  		} else allowedId = frontEast->id;
  	}
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

int main ( int argc, char *argv[] )
{
	int		trainCount = 0;
	char 		*filename = NULL;
	pthread_t	*tids;
	int		i;

		
	/* Parse the arguments */
	if ( argc < 2 )
	{
		printf ("Usage: part1 n {filename}\n\t\tn is number of trains\n");
		printf ("\t\tfilename is input file to use (optional)\n");
		exit(0);
	}
	
	if ( argc >= 2 )
	{
		trainCount = atoi(argv[1]);
	}
	if ( argc == 3 )
	{
		filename = argv[2];
	}	
	
	initTrain(filename);
	
	/*
	 * Since the number of trains to simulate is specified on the command
	 * line, we need to malloc space to store the thread ids of each train
	 * thread.
	 */
	tids = (pthread_t *) malloc(sizeof(pthread_t)*trainCount);
	
	/*
	 * Create all the train threads pass them the information about
	 * length and direction as a TrainInfo structure
	 */

	for (i=0;i<trainCount;i++)
	{
		TrainInfo *info = createTrain();
		printf ("Train %2d headed %s length is %d\n", info->trainId,
			(info->direction == DIRECTION_WEST ? "West" : "East"),
			info->length );

		if ( pthread_create (&tids[i],0, Train, (void *)info) != 0 )
		{
			printf ("Failed creation of Train.\n");
			exit(0);
		}
	}

	/*
	 * This code waits for all train threads to terminate
	 */
	for (i=0;i<trainCount;i++)
	{
		pthread_join (tids[i], NULL);
	}
	
	free(tids);
	return 0;
}