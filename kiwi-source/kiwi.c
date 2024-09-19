#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"
#define DATAS ("testdb")

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long double) tv.tv_sec + (long double) tv.tv_usec / 1000000.0;

}
//to open db
void _open_db(){
	db = db_open(DATAS); 
}
//to close db
void _close_db(){
	db_close(db);
}

void _write_test(long int count,int r, int timeflag)
{
	int i;
	Variant sk, sv;
	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];
	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);
	for (i = 0; i < count; i++) {
		if (r){
			_random_key(key, KSIZE);
		}else{
			snprintf(key, KSIZE, "key-%d", i);
		}
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);
		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;
		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", i, "");
			fflush(stderr);
		}
	}
	if (timeflag == 1)
		sum_of_cost[0] = get_time() - sum_of_cost[0];
	sum_of_counts[0]+=count;
}
void _read_test(long int count,int r, int timeflag)
{
	int i;
	int ret;
	Variant sk;
	Variant sv;
	char key[KSIZE + 1];
	for (i = 0; i < count; i++) {
		memset(key, 0, KSIZE + 1);
		/* if you want to test random write, use the following */
		if (r){
			_random_key(key, KSIZE);
		}else{
			snprintf(key, KSIZE, "key-%d", i);
		}
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);
		if (ret) {
			found_keys++;
		} else {
			INFO("not found key#%s", sk.mem);
    	}
		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", i, "");
			fflush(stderr);
		}
	}
	if (timeflag == 1)
		sum_of_cost[1] = get_time() - sum_of_cost[1];
	sum_of_counts[1]+=count;
}

void* request_thread(void* args) {
	threads_data* t_args = (threads_data*) args;
	if(t_args->read_or_write == 1){
		_write_test(t_args->count, t_args->r,t_args->timeflag);
		pthread_exit(NULL);
	}else if( t_args->read_or_write == 0){
		_read_test(t_args->count, t_args->r,t_args->timeflag);
		pthread_exit(NULL);
	}
	return NULL;
}

void _create_threads_test(long int count, long int threads, int r, double percentage) {
    
	pthread_t* thread_ids = (pthread_t*) malloc(threads * sizeof(pthread_t));
    threads_data* t_data = (threads_data*) malloc(threads * sizeof(threads_data));
    long int i;

    // Calculate the number of read and write operations based on the percentage
    long int total_write_count = (percentage>=0 && count>2 && percentage!=30000) ? count * percentage / 100 : (count/2); //total counts for write
    long int total_read_count = count - total_write_count; //total counts for read 
    
    //Calculate the threads for each operation
    long int write_threads = (percentage>=0 && threads!=2 && percentage!=30000) ? (threads*percentage/100): (threads/2); //total threads for write 
    long int read_threads = threads - write_threads ; //total threads for read
    
	threads_for_rw[0] = write_threads; //save the threads for write (for statisticc results)
    threads_for_rw[1] = read_threads; //save the threads for read (for statistic results)
    
	long int* counts_per_write = (long int*) malloc(sizeof(long int) * write_threads); // malloc to keep counts for every write thread
    long int* counts_per_read = (long int*) malloc(sizeof(long int) * read_threads); //malloc to keep counts for every read thread
    
    //split equal the counts in threads
    for(i = 0; i < write_threads; i++){
        counts_per_write[i]=(i<total_write_count%write_threads)?total_write_count/write_threads+1:total_write_count/write_threads;
    }
    for(i = 0; i < read_threads; i++){
        counts_per_read[i] = (i < total_read_count % read_threads) ? total_read_count / read_threads + 1 : total_read_count / read_threads;
    }
	
    // Create threads
    for (i = 0; i < threads; i++) {
        if (strcmp(choice, "readwrite") == 0) { //READWRITE
            if ((i % 2 == 0 && write_threads>=1) || (read_threads<=0 && write_threads>=1) ) { //read-write alternate and condiitons if someone ends first
                if(sum_of_cost[0] == 0) //if this is the first thread write start the clock
					sum_of_cost[0] =get_time(); 
				t_data[i].count = counts_per_write[write_threads-1]; //pass the counts
                t_data[i].read_or_write = 1; //write flag for request
				write_threads--; //-1 thread for write
            } else { // Assign read operation to the other half of the threads
                if(sum_of_cost[1] == 0) // //if this is the first thread read start the clock
					sum_of_cost[1] =get_time();
				t_data[i].count = counts_per_read[read_threads-1]; //pass the counts
                t_data[i].read_or_write = 0; //set the read flag for request
				read_threads--; //-1 thread for read
            }
			// update the end time
			if (write_threads == 0 && total_write_count!=0){ //when all write threads are done 
				t_data[i].timeflag = 1; //stop the time
				write_threads = -1; //to prevent the if statement
			}else if (read_threads == 0 && total_read_count!=0){ //the same for read threads
				t_data[i].timeflag = 1;
				read_threads = -1;
			}
        } else { //READ OR WRITE
            t_data[i].count = (i < count % threads) ? count / threads + 1 : count / threads; //split the counts equal to threads for read or write
            t_data[i].read_or_write = (strcmp(choice, "write") == 0) ? 1 : 0; //if write then set flag to 1 for the request else 0 for read
			//time check
			if(t_data[i].read_or_write == 1 && i==0) //if first thread is write then set the time 
				sum_of_cost[0] = get_time(); 
			else if(i==0) //else we have first thread a reader and set the clock
				sum_of_cost[1] = get_time();
            if(i==threads-1) //if the last thread comes then
                t_data[i].timeflag = 1; //stop clock 
        }

        t_data[i].r = r; //random keys
        pthread_create(&thread_ids[i], NULL, request_thread, (void*)&t_data[i]); //create thread
    }

    // join threads
    for ( i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL); 
    }
   
   // free memory
    free(thread_ids);
    free(t_data);
    free(counts_per_write);
    free(counts_per_read);
}
