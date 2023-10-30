/*
 * @autor Benjamin Deutsch (12215881)
 * @brief a module to compress strings
 * @date 30.10.2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/**
 * @return the number of digits of the parameter.
 */
static int get_digit_count(int number){
	int count = 0;
	if(number < 0){
		number = number * (-1);
	}
	if(number == 0){
		return 1;
	}
	
	while (number > 0) {
		number = number / 10;
		count++;
	}
	return count;
}

/**
 * @brief Compresses the input string by truncating substrings consisting of the same letter. 
 * @return the compressed version of the input string.
 */
/*@null@*/
static char *compress(const char *input){
	assert(input != NULL);
	//the maximum length of the result is the size of the input * 2
	char *result = (char *) malloc(strlen(input) * 2 + 1);
	if(result == NULL){
		printf("Memory allocation error\n");
		return NULL;
	}
	int result_index = 0;
	char last_char = input[0];
	int last_char_count = 1;
	
	bool exit = false;
	int i = 1;
	while(!exit){
		if(input[i] == '\0'){
			exit = true;
		}
		if(last_char == input[i] && input[i] != '\n'){
			last_char_count++;
		}else{
			//add character
			result[result_index] = last_char;
			result_index++;
			if(last_char != '\n'){
				//add number of occurences
				int digitCount = get_digit_count(last_char_count);
				sprintf(&result[result_index], "%d", last_char_count);
				result_index += digitCount;
			}
			last_char = input[i];
			last_char_count = 1;
		}
		i++;
	}
	result[result_index] = '\0';
	
	char *reallocated = (char *) realloc(result,strlen(result)+1);
	if(reallocated == NULL) {
		printf("Memory allocation error\n");
		free(result);
		return NULL;
	}
	return reallocated;
}

/**
 * @brief Reads the content of an arbitrarily large stream.
 * @return the content of the stream.
 */
/*@null@*/
static char *get_stream_content(FILE *stream) {
	char *input = (char *) malloc(1);
	if(input == NULL){
		printf("Memory allocation error\n");
		return NULL;
	}
	input[0] = '\0';
	int input_size = 0;
	char buffer[1000];
	
	while(fgets(buffer, (int) sizeof(buffer), stream) != NULL) {
		int new_size = input_size + strlen(buffer);
		char *new_input = (char *) realloc(input, (size_t) new_size+1);
		if(new_input == NULL){
			printf("Memory allocation error\n");
			free(input);
			return NULL;
		}
		
		input = new_input;
		strcat(input, buffer);
		input_size = new_size;
	}
	return input;
}

/**
 *	@brief Writes the given string to the file. If the file pointer is NULL the string is printed to stdout.
 */
static void write_to_output(/*@null@*/ FILE *file, char *str) {
	if(file == NULL){
		printf("%s", str);
	}else{
		fprintf(file, "%s", str);
	}
}

static void printUsageMessage(void) {
	printf("USAGE: mycompress [-o outfile] [file...]\n");
}

int main(int argc, char *argv[]){
	int c;
	char *outfilename = NULL;
	FILE *outfile = NULL;
	while((c = getopt(argc,argv,"o:")) != -1){
		switch(c){
			case 'o':
				if(outfilename != NULL) {
					printUsageMessage();
					return EXIT_FAILURE;
				}
				outfilename = optarg;
				break;
			case '?':
				printUsageMessage();
				return EXIT_FAILURE;
			default:
				assert(0);
		}
	}
	
	int infiles_count = argc - optind;
	int x = argc;
	char *infiles[infiles_count];
	int i;
	
	for(i = optind; i < x; i++) {
		infiles[i - optind] = argv[i];
	}
	
	if(outfilename != NULL) {
		outfile = fopen(outfilename, "w");
		if(outfile == NULL) {
			printf("An error occured trying to open the output file \"%s\".\n", outfilename);
			return EXIT_FAILURE;
		}
	}
		
	int readLength = 0;
	int writeLength = 0;
	if(infiles_count == 0){
		char *content = get_stream_content(stdin);
		if(content == NULL){
			free(content);
			return EXIT_FAILURE;
		}
		char *compressed = compress(content);
		if(compressed == NULL){
			free(content);
			free(compressed);
			return EXIT_FAILURE;
		}
		
		readLength += strlen(content);
		writeLength += strlen(compressed);
		write_to_output(outfile, compressed);
		
		free(content);
		free(compressed);
	}else{
		for(i = 0; i < infiles_count; i++){
			FILE *file = fopen(infiles[i], "r");
			if(file == NULL) {
				printf("An error occured trying to open the input file \"%s\".\n", infiles[i]);
				return EXIT_FAILURE;
			}
			
			char *content = get_stream_content(file);
			if(content == NULL){
				free(content);
				return EXIT_FAILURE;
			}
			char *compressed = compress(content);
			if(compressed == NULL){
				free(content);
				free(compressed);
				return EXIT_FAILURE;
			}
			
			readLength += strlen(content);
			writeLength += strlen(compressed);
			write_to_output(outfile, compressed);
			
			free(content);
			free(compressed);
			fclose(file);
		}
	}
	if(outfile != NULL){
		fclose(outfile);
	}
	
	fprintf(stderr, "Read: %d characters\nWritten: %d characters\n", readLength, writeLength);
	return EXIT_SUCCESS;
}
