#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "shared_buffer.h"

static bool quit = false;

/**
 * @brief prints the usage message for the supervisor
 */
static void print_usage_message(void) {
	printf("USAGE: supervisor [-n limit] [-w delay] [-p]\n");
}

/**
 * @brief handles a signal by setting the quit flag to true
 * @param signal the signal
 */
static void handle_signal(int signal) {
	quit = true;
}

/**
 * @brief returns the size of a feedback arc set solution
 * @details calclates the size of a feedback arc set solution by counting the occurences of the '-' character
 *          and returns the number of occurences.
 * @return the size of the solution
 */
static int get_solution_size(char *str){
	int i;
	int size = 0;
	for(i = 0; i < strlen(str); i++){
		if(str[i] == '-'){
			size++;
		}
	}
	return size;
}

int main(int argc, char *argv[]) {
	//add signal handlers
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	
	//read input
	int c, n = -1, w = 0, count;
	while((c = getopt(argc, argv, "n:w:p")) != -1){
		switch(c) {
			case 'n':
				count = sscanf(optarg, "%d", &n);
				if(count != 1 || n < 0){
					print_usage_message();
					return EXIT_FAILURE;
				}
				break;
			case 'w':
				count = sscanf(optarg, "%d", &w);
				if(count != 1 || w < 0){
					print_usage_message();
					return EXIT_FAILURE;
				}
				break;
			case 'p':
				break;
			default:
				print_usage_message();
				return EXIT_FAILURE;
				
		}
	}
	bool checkN = (n != -1);
	
	int shmfd = create_shared_memory();
	if(shmfd == -1){
		return EXIT_FAILURE;
	}
	shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if(shared_data == MAP_FAILED){
		close(shmfd);
		perror("supervisor: Shared memory mapping failed");
		return EXIT_FAILURE;
	}
	if (close(shmfd) == -1){
		perror("supervisor: Error closing shared memory file descriptor");
		return EXIT_FAILURE;
	}
	
	sleep(w);
	
	int best_size = -1;
	bool acyclic = false;
	int asdf = 0;
	while(!quit && !acyclic && (!checkN || n > 0)) {
		asdf++;
		char *str = read_solution(shared_data);
		int solution_size = get_solution_size(str);
		if(solution_size == 0) {
			printf("The graph is acyclic!\n");
			acyclic = true;
		}
		if(best_size == -1 || solution_size < best_size) {
			best_size = solution_size;
			//printf("Solution with %d edges: %s\n", best_size, str);
		}
		free(str);
		if(checkN){
			n--;
		}
	}
	shared_data->quit = true;
	if(!acyclic) {
		printf("The graph might not be acyclic, best solution removes %d edges.\n", best_size);
	}
	
	if(release_waiting_processes() == -1) {
		return EXIT_FAILURE;
	}
	if (munmap(shared_data, sizeof(shared_data_t)) == -1){
		perror("supervisor: Shared memory unmapping failed");
		return EXIT_FAILURE;
	}
	if(unlink_shared_memory() == -1) {
		return EXIT_FAILURE;
	};
	if(unlink_semaphores() == -1){
		return EXIT_FAILURE;
	}
	if(close_semaphores() == -1){
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

