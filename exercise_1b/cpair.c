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
#include <sys/wait.h>
#include <math.h>

typedef struct point {
	float x;
	float y;
} point_t;

typedef struct {
    point_t *array;
    size_t length;
} point_array_t;

typedef struct {
    point_array_t p1;
    point_array_t p2;
} splitted_point_array_t;

/**
 * @brief reads a list of points from a file or stdin
 * @param input specifies the file which should be read from, if null this function reads from stdin
 * @return the points array and its length
 */
static point_array_t readPoints(/*@null@*/FILE *input) {
	int arr_length = 2;
	point_array_t points;
	points.array = malloc(arr_length * sizeof(point_t));
	if(points.array == NULL) {
		printf("Memory allocation error\n");
		exit(EXIT_FAILURE);
	}
	points.length = 0;
	bool end = false;
	while(end == false) {
		int success;
		if(input == NULL){
			success = scanf("%f %f", &points.array[points.length].x, &points.array[points.length].y);
		}else{
			success = fscanf(input, "%f %f", &points.array[points.length].x, &points.array[points.length].y);
		}
		if(success != 2){
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
static splitted_point_array_t split_points_equally(point_array_t points) {
	splitted_point_array_t result;
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
static splitted_point_array_t split_points_by_threshold(point_array_t points, float threshold, bool splitByX) {
	splitted_point_array_t result;
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
static splitted_point_array_t split_points(point_array_t points) {
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
 * @param other_pipe_in both ends of this pipe are closed
 * @param other_pipe_out both ends of this pipe are closed
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

/**
 * @brief waits for a child process to be terminated, exits programm if the child did not terminate with status 0
 *	@param pid the process id of the child process
 */
static void wait_for_child(pid_t pid) {
	int status;
	waitpid(pid, &status, 0);
	if (WIFEXITED(status)) {
		if(WEXITSTATUS(status) != 0){
			printf("Child with PID %d exited with status %d\n", pid, WEXITSTATUS(status));
			exit(EXIT_FAILURE);
		}
	} else {
		printf("Child with PID %d terminated abnormally\n", pid);
		exit(EXIT_FAILURE);
	}
}

/**
 * @return the distance between the two parameters
 */
static double get_distance(point_t p1, point_t p2) {
	return sqrt(pow(p1.x-p2.x,2) + pow(p1.y-p2.y,2));
}

/**
 * @brief prints the two points to stdout in the following order: 
 *	the point with the lower x value first or if the x values are equal the point with the lower y value first
 *
 */
static void print_points(point_t p1, point_t p2) {
	if(p1.x > p2.x){
		point_t swap = p1;
		p1 = p2;
		p2 = swap;
	}else if(p1.x == p2.x) {
		if(p1.y > p2.y) {
			point_t swap = p1;
			p1 = p2;
			p2 = swap;
		}
	}
	
	
	printf("%.3f %.3f\n%.3f %.3f\n", p1.x, p1.y, p2.x, p2.y);
}

int main() {
	int i, j;
	point_array_t points = readPoints(NULL);
	if(points.length <= 0){
		free(points.array);
		return EXIT_FAILURE;
	}
	if(points.length == 1) {
		free(points.array);
		return EXIT_SUCCESS;
	}
	if(points.length == 2) {
		print_points(points.array[0],points.array[1]);
		free(points.array);
		return EXIT_SUCCESS;
	}
	splitted_point_array_t splitted = split_points(points);
	
	//initialize children and pipes
	int pipe_in1[2];
	int pipe_out1[2];
	int pipe_in2[2];
	int pipe_out2[2];
	if(pipe(pipe_in1) == -1 || pipe(pipe_out1) == -1 || pipe(pipe_in2) == -1 || pipe(pipe_out2) == -1) {
		printf("Pipe creation failed\n");
		return EXIT_FAILURE;
	}
	pid_t child_pid1 = init_child_process(pipe_in1, pipe_out1, pipe_in2, pipe_out2);
	pid_t child_pid2 = init_child_process(pipe_in2, pipe_out2, pipe_in1, pipe_out1);
	close(pipe_in1[0]);
	close(pipe_out1[1]);
	close(pipe_in2[0]);
	close(pipe_out2[1]);
	
	//give children their input
	FILE *file_in1 = fdopen(pipe_in1[1],"w");
	FILE *file_in2 = fdopen(pipe_in2[1],"w");
	if(file_in1 == NULL || file_in2 == NULL){
		perror("An error occured while trying to write to pipe");
		return EXIT_FAILURE;
	}
	
	for(i = 0; i < splitted.p1.length; i++){
		fprintf(file_in1,"%f %f\n", splitted.p1.array[i].x, splitted.p1.array[i].y);
	}
	for(i = 0; i < splitted.p2.length; i++){
		fprintf(file_in2,"%f %f\n", splitted.p2.array[i].x, splitted.p2.array[i].y);
	}
	fclose(file_in1);
	fclose(file_in2);
	
	wait_for_child(child_pid1);
	wait_for_child(child_pid2);
	
	//read childrens output
	FILE *file_out1 = fdopen(pipe_out1[0],"r");
	FILE *file_out2 = fdopen(pipe_out2[0],"r");
	point_array_t closest_pair1 = readPoints(file_out1);
	point_array_t closest_pair2 = readPoints(file_out2);
	fclose(file_out1);
	fclose(file_out2);
	
	//merge the results from the children
	bool free_splitted_p1 = true;
	bool free_splitted_p2 = true;
	if(closest_pair1.length > 0){
		free(splitted.p1.array);
		free_splitted_p1 = false;
		splitted.p1 = closest_pair1;
	}
	if(closest_pair2.length > 0){
		free(splitted.p2.array);
		free_splitted_p2 = false;
		splitted.p2 = closest_pair2;
	}
	int merged_length = splitted.p1.length + splitted.p2.length;
	point_t merged[merged_length];
	for(i = 0; i < splitted.p1.length; i++){
		merged[i] = splitted.p1.array[i];
	}
	for(i = 0; i < splitted.p2.length; i++){
		merged[i+splitted.p1.length] = splitted.p2.array[i];
	}
	
	//find the closest pair of points in the merged array
	double smallest_dist = -1;
	point_t p1;
	point_t p2;
	for(i = 0; i < merged_length; i++){
		for(j = i+1; j < merged_length; j++){
			double dist = get_distance(merged[i], merged[j]);
			if(dist < smallest_dist || smallest_dist == -1){
				p1 = merged[i];
				p2 = merged[j];
				smallest_dist = dist;
			}
		}
	}
	print_points(p1,p2);
	
	if(free_splitted_p1){
		free(splitted.p1.array);
	}
	if(free_splitted_p2){
		free(splitted.p2.array);
	}
	free(closest_pair1.array);
	free(closest_pair2.array);
	free(points.array);
	return EXIT_SUCCESS;
}
