/*
 * @autor Benjamin Deutsch (12215881)
 * @brief a module to find the closest pair of points from a list of points
 * @date 30.10.2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

typedef struct point {
	float x;
	float y;
} point_t;

typedef struct {
    point_t *array;
    size_t length;
} point_array;

typedef struct {
    point_array p1;
    point_array p2;
} splitted_point_array;

/**
 * @brief reads a list of points from stdin
 * @return the points array and its length
 */
static point_array readPoints() {
	int arr_length = 2;
	point_array points;
	points.array = malloc(arr_length * sizeof(point_t));
	if(points.array == NULL) {
		printf("Memory allocation error\n");
		exit(EXIT_FAILURE);
	}
	points.length = 0;
	bool end = false;
	while(end == false) {
		int succ = scanf("%f %f", &points.array[points.length].x, &points.array[points.length].y);	
		if(succ != 2){
			end = true;
		}else {
			getchar();
			points.length++;
			if(points.length >= arr_length){
				arr_length *= 2;
				point_t *reallocated = realloc(points.array, arr_length * sizeof(point_t));
				if(reallocated == NULL){
					printf("Memory allocation error\n");
					free(points.array);
					exit(EXIT_FAILURE);
				}
				points.array = reallocated;
			}
		}
	}
	return points;
}

/**
 * @brief splits a point array into two halves
 * @param points the points to be splitted
 *	@return the two halves and their lengths
 */
static splitted_point_array split_points_equally(point_array points) {
	splitted_point_array result;
	int i, mid = points.length/2;
	result.p1.length = mid;
	result.p1.array = malloc(result.p1.length * sizeof(point_t));
	result.p2.length = points.length - mid;
	result.p2.array = malloc(result.p2.length * sizeof(point_t));
	for(i = 0; i < points.length; i++) {
		if(i < mid){
			result.p1.array[i] = points.array[i];
		}else{
			result.p2.array[i-mid] = points.array[i];
		}
	}
	return result;
}

/**
 * @brief splits the points array into two arrays, one with the points that are below the threshold and another array with the rest.
 * @param points the points to be splitted
 * @param threshold the value that is used to divide the points into two arrays
 * @param splitByX if true, the x coordinates of the points will be compared to the threshold, otherwise the y coordinates
 */
static splitted_point_array split_points_by_threshold(point_array points, float threshold, bool splitByX) {
	splitted_point_array result;
	int i, n = 0;
	for(i = 0; i < points.length; i++) {
		if(points.array[i].y < threshold) {
			n++;
		}
	}
	result.p1.length = result.p2.length = 0;
	result.p1.array = malloc(n * sizeof(point_t));
	result.p2.array = malloc((points.length - n) * sizeof(point_t));
	for(i = 0; i < points.length; i++) {
		float val;
		if(splitByX) {
			val = points.array[i].x;
		}else{
			val = points.array[i].y;
		}
		if(val < threshold) {
			result.p1.array[result.p1.length] = points.array[i];
			result.p1.length++;
		}else{
			result.p2.array[result.p2.length] = points.array[i];
			result.p2.length++;
		}
	}
	return result;
}

/**
 * @brief splits an array of points into two arrays
 * @param points the points to be splitted
 */
static splitted_point_array split_points(point_array points) {
	int i;
	bool sameX = true;
	bool sameY = true;
	for(i = 0; i < points.length-1; i++) {
		if(points.array[i].x != points.array[i+1].x){
			sameX = false;
		}
	}
	if(sameX) {
		for(i = 0; i < points.length; i++) {
			if(points.array[i].y != points.array[i+1].y) {
				sameY = false;
			}
		}
	}
	if(sameX && sameY) {
		return split_points_equally(points);
	}
	if(sameX) {
		//split the array by mean y
		float meanY = 0;
		for(i = 0; i < points.length; i++){
			meanY += points.array[i].y;
		}
		meanY /= points.length;
		return split_points_by_threshold(points, meanY, false);
	}
	float meanX = 0;
	for(i = 0; i < points.length; i++){
		meanX += points.array[i].x;
	}
	meanX /= points.length;
	return split_points_by_threshold(points, meanX, true);
}

/**
 * @brief creates a child process, closes and redirects the according pipe ends
 * @param pipe_in stdin is redirected to the read end of this pipe
 * @param pipe_out stdout is rediredted to the write end of this pipe
 * @param other_pipe_in both ends are closed
 * @param other_pipe_out both ends are closed
 * @return the process id of the child process
 */
pid_t init_child_process(int *pipe_in, int* pipe_out, int* other_pipe_in, int* other_pipe_out) {
	pid_t child_pid = fork();
	if(child_pid < 0){
		printf("Fork failed\n");
		exit(EXIT_FAILURE);
	}
	if(child_pid == 0){
		//close all pipes from the other child process
		close(other_pipe_in[0]);
		close(other_pipe_in[1]);
		close(other_pipe_out[0]);
		close(other_pipe_out[1]);
		//close write end
		close(pipe_in[1]);
		//redirect stdin to read end
		dup2(pipe_in[0],0);
		close(pipe_in[0]);
		
		//close read end
		close(pipe_out[0]);
		//redirect stdout to write end
		dup2(pipe_out[1],1);
		close(pipe_out[1]);
		
		execlp("./cpair", "cpair", NULL);
		printf("Execution in child process failed\n");
		exit(EXIT_FAILURE);
	}
	//return the child process id in the parent process
	return child_pid;
}

int main() {
	point_array points = readPoints();
	if(points.length <= 1) {
		free(points.array);
		return EXIT_SUCCESS;
	}
	if(points.length == 2) {
		int first = 0, second = 1;
		if(points.array[0].x == points.array[1].x) {
			if(points.array[0].y > points.array[1].y) {
				first = 1;
				second = 0;
			}
		}else if(points.array[0].x > points.array[1].x){
			first = 1;
			second = 0;
		}
		printf("%f %f\n%f %f", points.array[first].x, points.array[first].y, points.array[second].x, points.array[second].y);
		free(points.array);
		return EXIT_SUCCESS;
	}
	splitted_point_array splitted = split_points(points);
	
	int pipe_in1[2];
	int pipe_out1[2];
	int pipe_in2[2];
	int pipe_out2[2];
	if(pipe(pipe_in1) == -1 || pipe(pipe_out1) == -1 || pipe(pipe_in1) == -1 || pipe(pipe_out2) == -1) {
		printf("Pipe creation failed\n");
		return EXIT_FAILURE;
	}
	
	pid_t child_pid1 = init_child_process(pipe_in1, pipe_out1, pipe_in2, pipe_out2);
	pid_t child_pid2 = init_child_process(pipe_in2, pipe_out2, pipe_in1, pipe_out1);
	
	close(pipe_in1[0]);
	close(pipe_out1[1]);
	close(pipe_in2[0]);
	close(pipe_out2[1]);
	
	int i;
	for(i = 0; i < splitted.p1.length; i++){
		
	}
	
	
	free(points.array);
	return EXIT_SUCCESS;
}
