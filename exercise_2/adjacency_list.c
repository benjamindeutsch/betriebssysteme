/*
 * @file
 * @brief adjecency_list module provides an implementation of an adjacency list to store a graph
 *
 * @autor Benjamin Deutsch (12215881)
 * @date 16.11.2023
 */
#include "adjacency_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
 * @brief checks whether the given list contains a vertex with the given id
 * @param list the list that the vertex should be searched in
 * @param vertex the vertex that should be searched for
 * @return true if the list contains the vertex, false otherwise
 */
static bool contains(adjacency_list_t *list, int vertex) {
	int i;
	for(i = 0; i < list->length; i++){
		if(list->vertices[i].id == vertex){
			return true;
		}
	}
	return false;
}

/*
 * @brief returns the vertex with the given id from the given list
 * @param list the list that the vertex should be searched in
 * @param vertex the vertex that should be searched for
 * @return NULL if the list does not contain a vertex with the given id, the vertex node with the given id otherwise
 */
static vertex_node_t* get_vertex_node(adjacency_list_t *list, int vertex) {
	int i;
	for(i = 0; i < list->length; i++){
		if(list->vertices[i].id == vertex){
			return &list->vertices[i];
		}
	}
	return NULL;
}

adjacency_list_t* create_list(void){
	int default_size = 2;
	adjacency_list_t *list = malloc(sizeof(adjacency_list_t));
	if(list == NULL){
		perror("adjecency_list: Memory allocation error occured.");
		return NULL;
	}
	list->size = default_size;
	list->length = 0;
	list->vertices = malloc(list->size * sizeof(vertex_node_t));
	if(list->vertices == NULL){
		free(list);
		perror("adjecency_list: Memory allocation error occured.");
		return NULL;
	}
	return list;
}

int add_vertex(adjacency_list_t *list, int vertex) {
	if(contains(list, vertex)){
		return 1;
	}
	//increase the size of the vertices array if necessary
	if(list->length + 1 > list->size){
		list->size = list->size * 2;
		vertex_node_t *reallocated = realloc(list->vertices, list->size * sizeof(vertex_node_t));
		if(reallocated == NULL){
			list->size = list->size / 2;
			perror("adjecency_list: Memory allocation error occured.");
			return -1;
		}
		list->vertices = reallocated;
	}
	//add the vertex	
	list->vertices[list->length].next = NULL;
	list->vertices[list->length].id = vertex;
	list->length = list->length + 1;
	return 0;
}

int add_edge(adjacency_list_t *list, int source_vertex, int destination_vertex){
	if(!contains(list, source_vertex) || !contains(list, destination_vertex)){
		return 1;
	}
	//create edge node
	vertex_node_t *edge_node = malloc(sizeof(vertex_node_t));
	if(edge_node == NULL){
		perror("adjecency_list: Memory allocation error occured.");
		return -1;
	}
	//insert edge node into the list of edges of the source vertex
	edge_node->id = destination_vertex;
	vertex_node_t *vertex = get_vertex_node(list, source_vertex);
	edge_node->next = vertex->next;
	vertex->next = edge_node;
	return 0;
}

void free_list_memory(adjacency_list_t *list) {
	int i;
	//free the edge lists
	for(i = 0; i < list->length; i++){
		while(list->vertices[i].next != NULL){
			vertex_node_t *next = list->vertices[i].next->next;
			free(list->vertices[i].next);
			list->vertices[i].next = next;
		}
	}
	//free vertices array
	free(list->vertices);
	free(list);
}

char* get_edges_string(adjacency_list_t *list) {
	int i, size = 4, buffSize = 100;
	char buff[100];
	char *str = malloc(size);
	if(str == NULL){
		perror("adjecency_list: Memory allocation error occured.");
		return NULL;
	}
	str[0] = '\0';
	
	for(i = 0; i < list->length; i++){
		vertex_node_t *next = list->vertices[i].next;
		while(next != NULL) {
			snprintf(buff, buffSize, "%d-%d ", list->vertices[i].id, next->id);
			if(strlen(str) + strlen(buff) + 1 > size){
				//size is not large enough, increase the size and loop again
				size = size * 2;
				char *reallocated = realloc(str, size);
				if(reallocated == NULL){
					free(str);
					perror("adjecency_list: Memory allocation error occured.");
					return NULL;
				}
				str = reallocated;
			}else{
				strcat(str,buff);
				next = next->next;
			}
		}
	}
	return str;
}

int main(int argc, char *argv[]) {
	adjacency_list_t *list = create_list();
	int i;
	for(i = 0; i < 10; i++){
		add_vertex(list,i);
	}
	for(i = 0; i < 10; i++){
		add_edge(list,i,(i+1)%10);
	}
	char *str = get_edges_string(list);
	printf("%s\n", str);
	
	free(str);
	free_list_memory(list);
}
