/**
 * @file
 * @brief HTTP Client module that partially implements the HTTP 1.1 standard
 *
 * @autor Benjamin Deutsch (12215881)
 * @date 23.12.2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/**
 * @brief prints the usage message for the http client to stdout and exits the programm with EXIT_FAILURE
 */
static void print_usage_message() {
	printf("SYNOPSIS: client [-p PORT] [ -o FILE | -d DIR ] URL\n");
	exit(EXIT_FAILURE);
}

/**
 * @brief prints the given error message to stderr and exits the programm with EXIT_FAILURE
 * @param msg the error message
 */
static void print_error_message(char *msg) {
	fprintf(stderr,"client: %s\n", msg);
	exit(EXIT_FAILURE);
}

/**
 * @brief extracts a substring from the input. The substring ends before a character of the delimiters string is encountered.
 * @param input the string from which the substring should be extracted from
 * @delimiters the substring ends before any of the characters of the delimiters string is encountered
 * @return the substring or NULL if a memory allocation error occured
 */
static char* extract_substring(char *input, char *delimiters) {
	char *end = strpbrk(input, delimiters);
	
	//if no delimiter is found, the end is the end of the url
	if (end == NULL) {
		end = (char *)input + strlen(input);
	}
	size_t length = end - input;
	char *substr = (char *)malloc(length + 1);
	if(substr == NULL) {
		perror("client: memory allocation error");
		return NULL;
	}
	strncpy(substr, input, length);
	substr[length] = '\0';
	
	return substr;
}

/**
 * @brief creates the content of a http get request for the given file of the given host
 * @param host the host to which the request should be made
 * @param file the requested file
 * @return the http get request content
 */
static char* get_http_get_request(char* host, char* file) {
	char *get_str = "GET ";
	char *http_str = " HTTP/1.1\r\n";
	char *host_str = "Host: ";
	char *connection_str = "\r\nConnection: close\r\n\r\n";
	
	char *request = malloc(strlen(get_str) + strlen(http_str) + strlen(host_str) + strlen(connection_str) + strlen(host) + strlen(file) + 1);
	if(request == NULL) {
		perror("client: memory allocation error");
		return NULL;
	}
	
	request[0] = '\0';
	strcat(request,get_str);
	strcat(request,file);
	strcat(request,http_str);
	strcat(request,host_str);
	strcat(request,host);
	strcat(request,connection_str);
	
	return request;
}

/**
 * @brief reads the content of a file and returns it as a string
 * @param file the file whose content should be read
 * @return the file content as a string
 */
static char* read_file_content(FILE *file) {
	size_t size = 1024;
	char buff[size];
	char *content = malloc(size);
	if(content == NULL){
		perror("client: memory allocation error");
		return NULL;
	}
	content[0] = '\0';
	while (fgets(buff, sizeof(buff), file) != NULL){
		if(size < strlen(content) + strlen(buff) + 1) {
			size = size * 2;
			char *reallocated = realloc(content, size);
			if(reallocated == NULL) {
				perror("client: memory allocation error");
				free(content);
				return NULL;
			}
			content = reallocated;
		}
		strcat(content,buff);
	}
	return content;
}

int main(int argc, char *argv[]) {
	char *port = "80";
	bool port_flag = false;
	char *out_filename = NULL, *out_dir = NULL, *url = NULL;
	int c;
	
	while((c = getopt(argc, argv, "p:o:d:")) != -1) {
		switch(c){
			case 'p':
				if(port_flag) {
					print_error_message("Invalid parameters");
				}
				port_flag = true;
				char *endptr;
				port = optarg;
				long port_num = strtol(port, &endptr, 10);
				
				if((errno == ERANGE && (port_num == LONG_MAX || port_num == LONG_MIN)) || 
					(errno == EINVAL && port_num == 0) || 
					port_num <= 0 || 
					port_num >= 65535 || 
					*endptr != '\0') {
					print_error_message("Invalid port");
				}
				break;
			case 'o':
				if(out_filename != NULL || out_dir != NULL) {
					print_error_message("Invalid parameters");
				}
				out_filename = optarg;
				break;
			case 'd':
				if(out_filename != NULL || out_dir != NULL) {
					print_error_message("Invalid parameters");
				}
				out_dir = optarg;
				break;
			default:
				return EXIT_FAILURE;
		}
	}
	
	//get host and file
	if(argc-optind != 1) {
		print_usage_message();
	}
	url = argv[optind];
	char* protocol = "http://";
	if(strlen(url) < strlen(protocol) || strncmp(url,protocol,strlen(protocol)) != 0){
		print_error_message("Invalid protocol");
	}
	
	char *host = extract_substring(url+strlen(protocol),";/?:@=&");
	if(host == NULL){
		return EXIT_FAILURE;
	}
	if(strlen(host) == 0){
		free(host);
		print_error_message("Invalid host");
	}
	
	char *filename;
	int file_start_index = strlen(protocol)+strlen(host);
	if(strlen(url) > file_start_index+1) {
		filename = extract_substring(url+file_start_index,"");
		if(filename == NULL){
			free(host);
			return EXIT_FAILURE;
		}
		//add / at the beginning of the filename
		if(filename[0] != '/') {
			char *tmp = malloc(strlen(filename)+2);
			if(tmp == NULL) {
				free(host);
				free(filename);
				perror("client: memory allocation error");
				return EXIT_FAILURE;
			}
			tmp[0] = '/';
			tmp[1] = '\0';
			strcat(tmp, filename);
			free(filename);
			filename = tmp;
		}
	}else {
		filename = (char *)malloc(2);
		if(filename == NULL){
			free(host);
			perror("client: memory allocation error\n");
			return EXIT_FAILURE;
		}
		filename[0] = '/';
		filename[1] = '\0';
	}
	char *http_request = get_http_get_request(host,filename);
	if(http_request == NULL){
		free(host);
		free(filename);
		return EXIT_FAILURE;
	}
	
	//create socket
	struct addrinfo hints, *ai;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	if(getaddrinfo(host, port, &hints, &ai) != 0) {
		free(filename);
		free(host);
		free(http_request);
		print_error_message("getaddrinfo error");
	}
	free(host);
	int sockfd = socket(ai->ai_family, ai->ai_socktype,ai->ai_protocol);
	if(sockfd < 0) {
		free(filename);
		free(http_request);
		perror("client: socket error");
		return EXIT_FAILURE;
	}
	if(connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0) {
		free(filename);
		free(http_request);
		close(sockfd);
		freeaddrinfo(ai);
		perror("client: connect error");
		return EXIT_FAILURE;
	}
	freeaddrinfo(ai);
	FILE *sockfile = fdopen(sockfd, "r+");
	if(sockfile == NULL){
		free(filename);
		free(http_request);
		close(sockfd);
		perror("client: fdopen error");
		return EXIT_FAILURE;
	}
	
	//send http request
	if(fputs(http_request, sockfile) == EOF) {
		free(filename);
		free(http_request);
		fclose(sockfile);
		print_error_message("socket fputs error");
	}
	free(http_request);
	if(fflush(sockfile) == EOF) {
		free(filename);
		fclose(sockfile);
		perror("client: fflush socket error");
		return EXIT_FAILURE;
	}
	
	//read response
	char *response = read_file_content(sockfile);
	fclose(sockfile);
	if(response == NULL){
		free(filename);
		return EXIT_FAILURE;
	}
	
	//check http response status
	char *response_copy = strdup(response);
	if(response_copy == NULL) {
		free(response);
		free(filename);
		perror("client: memory allocation error");
		return EXIT_FAILURE;
	}
	char *expected_http_token = "HTTP/1.1";
	char *http_token = strtok(response_copy, " ");
	char *status_string = strtok(NULL, " ");
	char *status_message = strtok(NULL, " ");
	char *endptr;
	long response_status = strtol(status_string, &endptr, 10);
	if((strlen(http_token) != strlen(expected_http_token) || strncmp(http_token,expected_http_token,strlen(expected_http_token)) != 0) ||
		((errno == ERANGE && (response_status == LONG_MAX || response_status == LONG_MIN)) || (errno == EINVAL && response_status == 0) || *endptr != '\0')) {
		free(response);
		free(filename);
		free(response_copy);
		fprintf(stderr, "Protocol error!");
		return EXIT_FAILURE;
	}
	free(response_copy);
	
	if(response_status != 200) {
		fprintf(stderr, "%ld %s\n", response_status, status_message);
		free(response);
		free(filename);
		return EXIT_FAILURE;
	}
	
	//output response
	char *content_start = "\r\n\r\n";
	char *content = strstr(response, content_start);
	if(content == NULL) {
		free(response);
		free(filename);
		fprintf(stderr, "Protocol error: no content!\n");
		return EXIT_FAILURE;
	}else{
		content = content + strlen(content_start);
	}
	
	FILE *out_file = stdout;
	if(out_filename != NULL) {
		out_file = fopen(out_filename, "w");
	}else if(out_dir != NULL) {
		char* filename_without_params = extract_substring(filename, "?");
		if(filename_without_params == NULL) {
			free(response);
			free(filename);
			return EXIT_FAILURE;
		}
		if(filename_without_params[0] == '/' && filename_without_params[1] == '\0') {
			char *default_filename = "/index.html";
			char *reallocated = realloc(filename_without_params,strlen(default_filename)+1);
			if(reallocated == NULL) {
				free(response);
				free(filename);
				free(filename_without_params);
				perror("client: memory allocation error");
				return EXIT_FAILURE;
			}
			filename_without_params = reallocated;
			filename_without_params[0] = '\0';
			strcat(filename_without_params, default_filename);
		}
		
		size_t size = strlen(out_dir) + strlen(filename_without_params) + 1;
		char full_filename[size];
		full_filename[0] = '\0';
		strcat(full_filename, out_dir);
		strcat(full_filename, filename_without_params);
		free(filename_without_params);
		
		out_file = fopen(full_filename, "w");
	}
	free(filename);
	if(out_file == NULL) {
		free(response);
		perror("client: fopen out file error");
		return EXIT_FAILURE;
	}
	
	fputs(content, out_file);
	free(response);
	if(out_file != stdout) {
		fclose(out_file);
	}
}
