#ifndef ADJACENCY_LIST_H
#define ADJACENCY_LIST_H
/*
 * @file
 * @brief adjecency_list module provides an implementation of an adjacency list to store a graph
 *
 * @autor Benjamin Deutsch (12215881)
 * @date 16.11.2023
 */
#include <stdbool.h>
#include <stdlib.h>

/*
 * @brief represents a vertex in the graph. Stores the edges that originate from the vertex.
 * @details represents the vertex with the given id in the graph. The next property is the head of a linked 
 *          list of vertices that the vertex is adjacent to and where the vertex is the origin of the edge.
 *
 */
typedef struct vertex_node {
	int id;
	struct vertex_node *next;
} vertex_node_t;

/*
 * @brief stores an array of vertex nodes, each of which stores the adjacent vertices, where the vertex is the origin.
 * @detail stores an array of vertex nodes, each of which stores the adjacent vertices, where the vertex is the origin.
 *         the length property represents the length of the list of vertices
 *         the size property represents the maximum number of vertices that can be stored in the list with the currently 
 *         allocated memory
 */
typedef struct adjacency_list {
	vertex_node_t *vertices;
	size_t length;
	size_t size;
} adjacency_list_t;


/*
 * @brief creates and initializes an empty list
 *	@return NULL if a memory allocation error occured, the list otherwise
 */
adjacency_list_t* create_list(void);

/*
 * @brief adds a vertex to the given adjacency list if it is not already contained in it
 * @details adds a vertex to the given adjacency list if there it does not contain a vertex with an id that is 
 *         equal to the id of the vertex to be inserted. Increases the size of the list if necessary.
 *	@param list the list that the vertex should be added to
 * @param vertex the vertex id of the new vertex
 * @return 0 if the vertex was added successfully, 
 *         1 if the vertex is already contained in the list, 
 *         -1 if a memory allocation error occured
 */
int add_vertex(adjacency_list_t* list, int vertex);

/*
 * @brief adds an edge between the two given vertices
 * @details adds an entry with the id destination_vertex to the list of adjacent vertices of the node representing 
 *         the vertex with the id source_vertex. The edge is not added if one of the given vertices does not exist.
 * @param list the list that the edge should be added to.
 * @param source_vertex the id of the vertex that the edge should originate from
 * @param destination_vertex the id of the vertex that where edge should end
 * @return 0 if the edge was added successfully, 
 *         1 if there is no vertex with the id source_vertex or if there is no vertex with the id destination_vertex, 
 *         -1 if a memory allocation error occured
 */
int add_edge(adjacency_list_t* list, int source_vertex, int destination_vertex);

/*
 * @brief returns a string representation of the edges in the graph
 * @return NULL if a memory allocation error occured, the string representation of the graph otherwise
 */
char* get_edges_string(adjacency_list_t* list);

/*
 * @brief frees all of the dynamically allocated memory of the given list.
 * @details frees all of the dynamically allocated memory of the given list. This includes the list itself.
 * @param list the list to be freed. This parameter has to be dynamically allocated
 */
void free_list_memory(adjacency_list_t* list);

#endif /* ADJACENCY_LIST_H */

