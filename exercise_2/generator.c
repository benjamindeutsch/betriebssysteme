/**
 * @file
 * @brief generator module to generate random solutions for the arc set problem
 *
 * @autor Benjamin Deutsch (12215881)
 * @date 16.11.2023
 */
#include "adjacency_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include "shared_buffer.h"

/**
 * @brief shuffles the vertices of the adjacency list randomly
 * @details implements the Fisher–Yates shuffle to reorder the items of the vertices array 
 *          of the given adjacency list randomly
 * @param list the list whose vertices should be shuffled randomly
 */
static void shuffle(adjacency_list_t *list) {
	//seed the random number generator with the current time
	int i;
	for(i = list->length-1; i > 0; i--) {
		//generate a random number x <= i
		int r = rand();
		int x = r % i;
		vertex_node_t swap = list->vertices[i];
		list->vertices[i] = list->vertices[x];
		list->vertices[x] = swap;
	}
}

int main(int argc, char *argv[]) {
	if(argc <= 1){
		fprintf(stderr, "generator: No input provided\n");
		return EXIT_FAILURE;
	}
	
	//link shared memory
	int shmfd = init_shared_memory();
	if(shmfd == -1){
		return EXIT_FAILURE;
	}
	shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if(shared_data == MAP_FAILED){
		close(shmfd);
		perror("generator: Shared memory mapping failed");
		return EXIT_FAILURE;
	}
	if (close(shmfd) == -1){
		perror("generator: Error closing shared memory file descriptor");
		return EXIT_FAILURE;
	}
	
	//read list from input
	int i;
	adjacency_list_t *list = create_list();
	for(i = 1; i < argc; i++){
		int v1, v2;
		int result = sscanf(argv[i],"%d-%d", &v1, &v2);
		if(result != 2){
			free_list_memory(list);
			fprintf(stderr, "generator: Invalid input\n");
			return EXIT_FAILURE;
		}else{
			if(!contains(list,v1)){
				add_vertex(list,v1);
			}
			if(!contains(list,v2)){
				add_vertex(list,v2);
			}
			add_edge(list,v1,v2);
		}
	}
	//generate feedback arcs
	srand(time(NULL));
	bool quit = false;
	while(!quit) {
		shuffle(list);
		adjacency_list_t *feedback_arc = create_list();
		//add vertices to feedback_arc list
		for(i = 0; i < list->length; i++) {
			add_vertex(feedback_arc,list->vertices[i].id);
		}
		//add edges
		int edgeCount = 0;
		for(i = 0; i < list->length; i++){
			vertex_node_t *next = list->vertices[i].next;
			while(next != NULL){
				int index = index_of(list, next->id);
				assert(index >= 0);
				if(i < index) {
					add_edge(feedback_arc, list->vertices[i].id, list->vertices[index].id);
					edgeCount++;
				}
				next = next->next;
			}
		}
		
		if(edgeCount <= 8) {
			char *str = get_edges_string(feedback_arc);
			if(shared_data->quit) {
				printf("quit received\n");
				quit = true;
			}else{
				write_solution(shared_data, str);
			}
			free(str);
		}
		free_list_memory(feedback_arc);
	}
	
	if (munmap(shared_data, sizeof(shared_data_t)) == -1){
		perror("supervisor: Shared memory unmapping failed");
		return EXIT_FAILURE;
	}
	if(close_semaphores() == -1){
		return EXIT_FAILURE;
	}
	free_list_memory(list);
	return EXIT_SUCCESS;
}
