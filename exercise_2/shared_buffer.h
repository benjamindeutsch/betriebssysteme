/**
 * @file
 * @brief shared_buffer module provides functions to manage shared memory containing a circular buffer
 *
 * @autor Benjamin Deutsch (12215881)
 * @date 16.11.2023
 */
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>

#define DATA_SIZE 20
#define DATA_ENTRY_SIZE 180 //large enough to store 8 pairs of ints (16 * 10 digits + 8 * "-" + 7 * " ")

typedef struct shared_data {
	char solutions[DATA_SIZE][DATA_ENTRY_SIZE];
	int write_pos;
	int read_pos;
	bool quit;
} shared_data_t;

/**
 * @brief creates and intializes shared memory and creates the semaphores for shared memory access.
 * @details creates shared memory and sets its size to the size of the shared_data struct. The semaphores are created
 *          with the corresponding start values. Should only be called once.
 * @return the file descriptor of the shared memory or -1 if an error occured
 */
int create_shared_memory(void);

/**
 *	@brief intializes shared memory and opens the semaphores for shared memory access.
 * @details initializes shared memory that has already been created. Opens the semaphores, which also have to be created
 *          before this function is called.
 * @return the file descriptor of the shared memory or -1 if an error occured
 */
int init_shared_memory(void);

/**
 * @brief unlinks the shared memory
 * @return 0 if unlinking the shared memory was successfull, -1 otherwise
 */
int unlink_shared_memory(void);

/**
 * @brief unlinks all of the semaphores that were created for the shared memory access
 * @return 0 if unlinking the semaphores was successfull, -1 otherwise
 */
int unlink_semaphores(void);

/**
 * @brief releases all of the processes waiting for the semaphores by calling sem_post unit its value is non-negative
 * @return 0 if all the semaphores are non-negative, -1 if an error occured
 */
int release_waiting_processes(void);

/**
 * @brief closes all of the semaphores that were created for the shared memory access
 * @return 0 if closing the semaphores was successfull, -1 otherwise
 */
int close_semaphores(void);

/**
 * @brief writes a solution to the solutions buffer if the quit flag is not true
 * @details writes a solution to the solutions circular buffer. This function may have to wait for data to be read in order
 *          to be able to write new data into the buffer. If the quit flag is true this function does not write to the buffer.
 * @param data the shared data struct that should be accessed
 * @param solution a string representation of a feedback arc set
 * @return true if the process calling this method should quit, false otherwise
 */
bool write_solution(shared_data_t* data, char *solution);

/**
 * @brief reads a solution from the solutions buffer
 * @details reads a solution from the solutions  circular buffer. This function may have to wait for new data to be written in order
 *          to be able read from the buffer.
 * @param data the shared data struct that should be accessed
 * @return a string representation of a feedback arc set or NULL if an error occured
 */
char* read_solution(shared_data_t* data);

#endif
