#include "shared_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>

const char SHARED_MEMORY_KEY[] = "feedback_arc_set_shm";
const char WRITE_SEMAPHORE_KEY[] = "feedback_arc_set_buffer_write";
const char BUFFER_FREE_SEMAPHORE_KEY[] = "feedback_arc_set_buffer_free";
const char BUFFER_USED_SEMAPHORE_KEY[] = "feedback_arc_set_buffer_used";
sem_t *write_sem = NULL, *free_sem = NULL, *used_sem = NULL;

int create_shared_memory(void) {
	//create or open shared memory
	int shmfd = shm_open(SHARED_MEMORY_KEY, O_RDWR | O_CREAT, 0600);
	if(shmfd == -1){
		perror("shared_buffer: Error opening shared memory.");
		return -1;
	}
	//set shared memory size
	if(ftruncate(shmfd, sizeof(shared_data_t)) < 0){
		close(shmfd);
		perror("shared_buffer: Error initializing shared memory.");
		return -1;
	}
	
	write_sem = sem_open(WRITE_SEMAPHORE_KEY, O_CREAT | O_EXCL, 0600, 1);
	if(write_sem == SEM_FAILED){
		close(shmfd);
		perror("shared_buffer: Error creating semaphore.");
		return -1;
	}
	free_sem = sem_open(BUFFER_FREE_SEMAPHORE_KEY, O_CREAT | O_EXCL, 0600, DATA_SIZE);
	if(free_sem == SEM_FAILED){
		close(shmfd);
		sem_close(write_sem);
		perror("shared_buffer: Error creating semaphore.");
		return -1;
	}
	used_sem = sem_open(BUFFER_USED_SEMAPHORE_KEY, O_CREAT | O_EXCL, 0600, 0);
	if(used_sem == SEM_FAILED){
		close(shmfd);
		sem_close(write_sem);
		sem_close(free_sem);
		perror("shared_buffer: Error creating semaphore.");
		return -1;
	}
	
	return shmfd;
}

int init_shared_memory(void) {
	int shmfd = shm_open(SHARED_MEMORY_KEY, O_RDWR, 0600);
	if(shmfd == -1){
		perror("shared_buffer: Error opening shared memory.");
		return -1;
	}
	
	write_sem = sem_open(WRITE_SEMAPHORE_KEY, O_RDWR);
	if(write_sem == SEM_FAILED){
		close(shmfd);
		perror("shared_buffer: Error opening semaphore.");
		return -1;
	}
	free_sem = sem_open(BUFFER_FREE_SEMAPHORE_KEY, O_RDWR);
	if(free_sem == SEM_FAILED){
		close(shmfd);
		sem_close(write_sem);
		perror("shared_buffer: Error opening semaphore.");
		return -1;
	}
	used_sem = sem_open(BUFFER_USED_SEMAPHORE_KEY, O_RDWR);
	if(used_sem == SEM_FAILED){
		close(shmfd);
		sem_close(write_sem);
		sem_close(free_sem);
		perror("shared_buffer: Error opening semaphore.");
		return -1;
	}
	
	return shmfd;
}

int unlink_shared_memory(void) {
	if(shm_unlink(SHARED_MEMORY_KEY) == -1) {
		perror("shared_buffer: Error unlinking shared memory.");
		return -1;
	}
	return 0;
}

int unlink_semaphores(void) {
	if(sem_unlink(WRITE_SEMAPHORE_KEY) == -1){
		perror("shared_buffer: Error unlinking write semaphore.");
		return -1;
	}
	if(sem_unlink(BUFFER_FREE_SEMAPHORE_KEY) == -1){
		perror("shared_buffer: Error unlinking free semaphore.");
		return -1;
	}
	if(sem_unlink(BUFFER_USED_SEMAPHORE_KEY) == -1){
		perror("shared_buffer: Error unlinking used semaphore.");
		return -1;
	}
	return 0;
}

int close_semaphores(void) {
	if(sem_close(write_sem) == -1){
		perror("shared_buffer: Error closing write semaphore.");
		return -1;
	}
	if(sem_close(free_sem) == -1){
		perror("shared_buffer: Error closing free semaphore.");
		return -1;
	}
	if(sem_close(used_sem) == -1){
		perror("shared_buffer: Error closing used semaphore.");
		return -1;
	}
	return 0;
}

void write_solution(shared_data_t* data, char *solution){
	sem_wait(free_sem);
	sem_wait(write_sem);
	strncpy(data->solutions[data->write_pos],solution,DATA_ENTRY_SIZE);
	data->write_pos = (data->write_pos + 1) % DATA_SIZE;
	sem_post(write_sem);
	sem_post(used_sem);
}

char* read_solution(shared_data_t* data){
	char *str = malloc(DATA_ENTRY_SIZE);
	if(str == NULL){
		perror("shared_buffer: Memory allocation error.");
		return NULL;
	}
	sem_wait(used_sem);
	strncpy(str, data->solutions[data->read_pos],DATA_ENTRY_SIZE);
	sem_post(free_sem);
	data->read_pos = (data->read_pos + 1) % DATA_SIZE;
	return str;
}
