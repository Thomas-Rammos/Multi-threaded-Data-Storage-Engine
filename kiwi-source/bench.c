#include "bench.h"
#include "../engine/db.h"
#include <time.h>

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";
	//char salt[6]= "abcdef";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}


 // FUNCTION FOR STATISTICS
int prints_for_results(long double total_time_write,long double total_time_read, long int threads,long int counts,double percentage,double total_cost)
{
	printf(LINE);
	printf("\n-------------STATISTICS-----------------\n");
	printf(" Benchmark mode -> %s \n Total threads -> %ld\n Total Counts -> %ld\n Percentage -> %.2f/100.00\n Total bench cost -> %f\n",
			choice,threads,counts,percentage,total_cost);
	
	//print statistics for write
	if (strcmp(choice, "write")==0) 
	{
		printf(LINE);
		printf("------------- RESULTS -------------\n");
		printf("a)Writes -> done:%ld\nb)Seconds/operation -> %.6Lf sec/op \nc)Writes/second(estimated) -> %.1Lf writes/sec \nd)Threads -> %ld\ne)Seconds/Thread(estimated) -> %Lf sec/th\nf)Total cost -> %Lf sec\n ",
			sum_of_counts[0],
			total_time_write / sum_of_counts[0],
			sum_of_counts[0] / total_time_write,
			threads,
			total_time_write / threads,
			total_time_write);
	}
	//print statistics for read
	else if (strcmp(choice, "read")==0) 
	{
		printf(LINE);
		printf("------------- RESULTS -------------\n");
		printf("a)Reads -> done:%ld\nb)Found -> %ld\nc)Seconds/operation -> %.6Lf sec/op \nd)Reads/second(estimated) -> %.1Lf read/sec \ne)Threads -> %ld\nf)Seconds/Thread(estimated) -> %Lf sec/th\ng)Total cost -> %Lf sec\n",
			sum_of_counts[1],found_keys,
			total_time_read / sum_of_counts[1],
			sum_of_counts[1] / total_time_read,
			threads,
			total_time_read / threads,
			total_time_read);
	//print statistics for readwrite		
	}else if (strcmp(choice, "readwrite")==0){
		printf(LINE);
		printf("------------- WRITE RESULTS -------------\n");
		printf("a)Writes -> done:%ld\nb)Seconds/operation -> %.6Lf sec/op \nc)Writes/second(estimated) -> %.1Lf writes/sec \nd)Threads -> %ld\ne)Seconds/Thread(estimated) -> %Lf sec/th\nf)Total cost -> %Lf sec\n ",
			sum_of_counts[0],
			total_time_write / sum_of_counts[0],
			sum_of_counts[0] / total_time_write,
			threads_for_rw[0],
			total_time_write / threads_for_rw[0],
			total_time_write);
		printf(LINE);
		//printf(LINE);
		printf("------------- READ RESULTS -------------\n");
		printf("a)Reads -> done:%ld\nb)Found -> %ld\nc)Seconds/operation -> %.6Lf sec/op \nd)Reads/second(estimated) -> %.1Lf read/sec \ne)Threads -> %ld\nf)Seconds/Thread(estimated) -> %Lf sec/th\ng)Total cost -> %Lf sec\n",
			sum_of_counts[1],found_keys,
			total_time_read / sum_of_counts[1],
			sum_of_counts[1] / total_time_read,
			threads_for_rw[1],
			total_time_read / threads_for_rw[1],
			total_time_read);
	}	
	return 1;
}

int main(int argc, char** argv) {
    if (argc < 3 || argc > 6) {
        fprintf(stderr, "Usage: db-bench <write|read|readwrite> <count> [<threads> <percentage> <random>]\n");
        exit(1);
    }

    strcpy(choice, argv[1]); //choice option (read,write,readwrite)
    long int count =  atol(argv[2]); //total counts 
    long int threads = argc >= 4 ? atol(argv[3]) : 1;  //total threads
    double percentage = argc >= 5 ? atof(argv[4]): 30000; //percentage for write , default value 30000
    int r = argc >= 5 ? 1 : 0; //random keys-value for read or write 
	double total_cost = get_time(); //total benchmark time 
    
	if (count <= 0 || threads <= 0 || percentage < 0 || (percentage> 100 && percentage != 30000)) {
        perror("Error Please try again but check the following reasons.\na)Negative or zero number of counts\nb)Negative or zero number of threads\nc)Percentage bigger than 100 or smaller or equal tο 0.\n");
        exit(1);
    }
	
    _print_header(count); //different headers about the size
    _print_environment(); //different enviroment variables about system
	_open_db(); //open the database
	
	//choose benchmark mode
    if (strcmp(choice, "write") == 0 && argc <= 5) {
		_create_threads_test(count, threads, r, percentage); //create thread system benchmark or multithreading system benchmark for write 
	}else if (strcmp(choice, "read") == 0  && argc <= 5) {
		_create_threads_test(count, threads, r, percentage); //create thread system benchmark or multithreading system benchmark for read 
	}else if (strcmp(choice, "readwrite") == 0) {
		if (argc == 3){
			threads = 2; //default value for threads in readwrite benchmark if the user doesn't give us threads
			_create_threads_test(count, threads, r, percentage); //create multitheading system benchmark for readwrite
		}else{
			if(percentage > 0 && argc >= 6 ) //if we have 6 arguements in readwrite it means random key values
				r = 1;
			else
				r = 0; //else don;t use random keys 
			_create_threads_test(count, threads, r, percentage); //create the multithrading system benchmark for readwrite
		}
	 }else{
		fprintf(stderr, "Invalid test type.\n");
		exit(1);
		}
	
	_close_db(); //close db
	total_cost = get_time()-total_cost; //stop the benchmark clock
	//print the resluts after the multithreading benchmark
	if (percentage == 30000) //if default percentage that means, the user doesn't give any percentage for the bench
		percentage = 0; // so update the value to 0 for the print porpuses
	prints_for_results(sum_of_cost[0],sum_of_cost[1],threads,count,percentage,total_cost); 
	
	return 1;
}
