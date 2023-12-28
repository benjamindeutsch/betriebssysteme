/**
 * @file
 * @brief HTTP Server module that partially implements the HTTP 1.1 standard
 *
 * @author Benjamin Deutsch (12215881)
 * @date 23.12.2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>

static bool quit = false;
static bool waiting = true;
static int sockfd = -1;

/**
 * @brief closes the socket file descriptor if it is open
 */
static void cleanup(void) {
	if(sockfd != -1) {
		close(sockfd);
	}
}

/**
 * @brief handles a signal by setting the quit flag to true
 * @param signal the signal
 */
static void handle_signal(int signal) {
	if(waiting) {
		cleanup();
		exit(EXIT_SUCCESS);	
	}
	quit = true;
}

/**
 * @brief prints the usage message for the http server to stdout and exits the programm with EXIT_FAILURE
 */
static void exit_with_usage_message(void) {
	printf("SYNOPSIS: client [-p PORT] [ -o FILE | -d DIR ] URL\n");
	exit(EXIT_FAILURE);
}

/**
 * @brief prints an error message to stderr
 * @param programm_name the name of the programm (argv[0])
 * @param msg the error message
 */
static void error_message(char *programm_name, char *msg) {
	fprintf(stderr,"%s: %s\n", programm_name, msg);
}

/**
 * @brief prints the given error message to stderr and exits the programm with EXIT_FAILURE
 * @param programm_name the name of the programm (argv[0])
 * @param msg the error message
 */
static void exit_with_error_message(char *programm_name, char *msg) {
	error_message(programm_name, msg);
	cleanup();
	exit(EXIT_FAILURE);
}

/**
 * @brief returns the value of the Content-Length header
 * @details returns the value of the Content-Length header. If the header is not found or the value of the header is not a number, 0 is returned.
 * @param headers the http headers
 * @return the value of the Content-Length header
 */
unsigned long get_content_length(const char *headers) {
	const char *search_str = "Content-Length: ";
	const size_t search_str_len = strlen(search_str);

	const char *start = strstr(headers, search_str);
	if (start == NULL) {
		return 0;  // Content-Length header not found
	}

	start += search_str_len;
	char *endptr;
	unsigned long length = strtoul(start, &endptr, 10);
	if (start == endptr) {
		return 0;
	}

	return length;
}

/**
 * @brief reads the content of a file and returns it as a string
 * @param programm_name the name of the programm (argv[0])
 * @param file the file whose content should be read
 * @param stopAtEmptyLine if true, then this function stops reading as soon as a an empty line ("\r\n") is encountered
 * @return the file content as a string
 */
static char* read_file_content(char *programm_name, FILE *file, bool stopAtEmptyLine) {
	size_t size = 1024;
	char buff[size];
	char *content = malloc(size);
	if(content == NULL){
		error_message(programm_name,"memory allocation error");
		return NULL;
	}
	content[0] = '\0';
	while (fgets(buff, sizeof(buff), file) != NULL){
		if(size < strlen(content) + strlen(buff) + 1) {
			size = size * 2;
			char *reallocated = realloc(content, size);
			if(reallocated == NULL) {
				error_message(programm_name,"memory allocation error");
				free(content);
				return NULL;
			}
			content = reallocated;
		}
		strcat(content,buff);
		if(stopAtEmptyLine && strcmp(buff, "\r\n") == 0) {
			break;
		}
	}
	return content;
}

/**
 * @brief sends a http response
 * @details writes an http response with the given status, status message and content to the given file. The file is closed after this function executed.
 * @param connfile the file to which the response should be written to
 * @param programm_name the name of the programm (argv[0])
 * @param status the response status
 * @param status_message the response status message
 * @param content the content of the response
 * @return true if the response has been written to the file successfully, false otherwise
 */
static bool send_response(FILE *connfile, char *programm_name, char *status, char *status_message, char *content) {
	bool statusIs200 = false;
	if(strcmp(status, "200") == 0) {
		statusIs200 = true;
	}
	char *http_str = "HTTP/1.1 ";
	char *connection_str = "Connection: close\r\n\r\n";
	char *content_length_label_str = "Content-Length: ";
	char content_length_str[30];
	snprintf(content_length_str, sizeof(content_length_str), "%ld", strlen(content)); 
	char *date_label_str = "Date: ";
	// Get the current time
	time_t current_time;
	struct tm *local_time;
	char date_str[80];
	current_time = time(NULL);
	local_time = localtime(&current_time);
	strftime(date_str, sizeof(date_str), "%a, %d %b %y %H:%M:%S %Z", local_time);
	
	char response[strlen(http_str) + strlen(status) + strlen(status_message) + 3 + strlen(date_label_str) + strlen(date_str) + 2 + strlen(content_length_label_str) + strlen(content_length_str) + 2 + strlen(connection_str) + strlen(content) + 20];
	
	response[0] = '\0';
	strcat(response, http_str);
	strcat(response, status);
	strcat(response, " ");
	strcat(response, status_message);
	strcat(response, "\r\n");
	if(statusIs200) {
		strcat(response, date_label_str);
		strcat(response, date_str);
		strcat(response, "\r\n");
		strcat(response, content_length_label_str);
		strcat(response, content_length_str);
		strcat(response, "\r\n");
	}
	strcat(response, connection_str);
	strcat(response, content);
	strcat(response, "\r\n\r\n");
	
	if(fputs(response, connfile) == EOF) {
		error_message(programm_name, "fputs response error");
		fclose(connfile);
		return false;
	}
	
	if(fflush(connfile) == EOF) {
		perror("fflush");
		error_message(programm_name, "fflush response error");
		fclose(connfile);
		return false;
	}
	
	if(fclose(connfile) == EOF) {
		error_message(programm_name, "fclose response error");
		return false;
	}
	return true;
}

int main(int argc, char *argv[]) {
	//add signal handlers
	struct sigaction siga;
	memset(&siga, 0, sizeof(siga));
	siga.sa_handler = handle_signal;
	sigaction(SIGINT, &siga, NULL);
	sigaction(SIGTERM, &siga, NULL);

	//get the programm arguments
	char *programm_name = argv[0], *index_filename = "/index.html", *doc_root;
	long port = 8080;
	bool port_flag = false, index_filename_flag = false;
	int c;
	while((c = getopt(argc, argv, "p:i:")) != -1) {
		switch(c){
			case 'p':
				if(port_flag) {
					exit_with_error_message(programm_name,"Invalid options");
				}
				char *endptr;
				port = strtol(optarg, &endptr, 10);
				
				if((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) || 
					(errno == EINVAL && port == 0) || 
					port <= 0 || 
					port >= 65535 || 
					*endptr != '\0') {
					exit_with_error_message(programm_name,"Invalid port");
				}
				port_flag = true;
				break;
			case 'i':
				if(index_filename_flag) {
					exit_with_error_message(programm_name, "Invalid options");
				}
				index_filename_flag = true;
				index_filename = optarg;
				break;
			default:
				exit_with_error_message(programm_name, "Invalid options");
		}
	}
	
	if(argc - optind != 1) {
		exit_with_usage_message();
	}
	doc_root = argv[optind];	
	
	//create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		exit_with_error_message(programm_name, "Socket error");
	}
	
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = INADDR_ANY;
	if (bind(sockfd, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0) {
		exit_with_error_message(programm_name, "Bind error");
	}
	
	if (listen(sockfd, 1) < 0) {
		exit_with_error_message(programm_name, "Listen error");
	}
	
	while(!quit) {
		waiting = true;
		int connfd = accept(sockfd, NULL, NULL);
		waiting = false;
		if(connfd < 0) {
			exit_with_error_message(programm_name, "Accept error");
		}
		FILE *connfile = fdopen(connfd, "r+");
		if(connfile == NULL) {
			exit_with_error_message(programm_name, "Fdopen error");
		}
		
		char *request = read_file_content(programm_name, connfile, true);
		if(request == NULL) {
			fclose(connfile);
			exit_with_error_message(programm_name, "read request error");
		}
		
		unsigned long content_length = get_content_length(request);
		unsigned long i;
		for(i = 0; i < content_length; i++) {
			fgetc(connfile);
		}
		
		char *http_method = strtok(request, " ");
		char *request_path = strtok(NULL, " ");
		char *http_token = strtok(NULL, "\r\n");
		if(http_method == NULL || request_path  == NULL || http_token == NULL || strcmp(http_token, "HTTP/1.1") != 0) {
			free(request);
			send_response(connfile, programm_name, "400", "(Bad Request)", "");
			continue;
		}
		if(strcmp(http_method, "GET") != 0) {
			free(request);
			send_response(connfile, programm_name, "501", "(Not Implemented)", "");
			continue;
		}
		if(strcmp(request_path, "/") == 0) {
			request_path = index_filename;
		}
		
		//open file
		char full_file_path[strlen(doc_root) + strlen(request_path) + 4];
		full_file_path[0] = '\0';
		strcat(full_file_path, "./");
		strcat(full_file_path, doc_root);
		if(request_path[0] != '/') {
			strcat(full_file_path, "/");
		}
		strcat(full_file_path, request_path);
		free(request);
		
		FILE *response_file = fopen(full_file_path, "r");
		if(response_file == NULL) {
			send_response(connfile, programm_name, "404", "(Not Found)", "");
			continue;
		}
		
		char *response_content = read_file_content(programm_name, response_file, false);
		if(response_content == NULL) {
			send_response(connfile, programm_name, "404", "(Not Found)", "");
		}else{
			send_response(connfile, programm_name, "200", "OK", response_content);
			free(response_content);
		}
		fclose(response_file);
	}
	cleanup();
}
