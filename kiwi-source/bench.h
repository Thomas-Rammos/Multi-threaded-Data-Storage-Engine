#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "../engine/db.h"
#define KSIZE (36)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);

//struct to keep the counts,r and a flag for the type of operation for every thread.
typedef struct{
	long int count; //counts for the thread
	int r;  //random keys
	int read_or_write; //if it is read(0) or write(1) thread
	int timeflag; //time flag 
}threads_data;

double get_time(); //get time in sec
void _create_threads_test(long int count,long int threads,int r,double percentage); //for the multithreading creation system

DB* db; //pointer for db.
char choice[10]; //to choose operation 
long int found_keys; // how many keys have been found from read
long int sum_of_counts[2];// usefull for the statistics for the counts of write[0] and read[1] 
long double sum_of_cost[2]; //statistic for costs ( write[0] and read[1] )
long int threads_for_rw[2];//threads for read and for write