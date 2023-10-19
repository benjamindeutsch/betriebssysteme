#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

int get_digit_count(int number){
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

char* compress(char *input){
	assert(input != NULL);
	//the maximum length of the result is the size of the input * 2
	int length = strlen(input);
	char *result = (char *) malloc(length * sizeof(char) * 2);
	int last_char = input[0];
	int last_char_count = 1;
	int i = 1;
	int result_index = 0;
	bool exit = false;
	
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
	/*int result_length = result_index + 2;
	return realloc(result,result_length * sizeof(char));*/
	return result;
}

char* get_file_content(char* filename) {
	FILE *file = fopen(filename, "r");
	if(file == NULL){
		printf("Could not open %s.\n", filename);
		return NULL;
	}
	fseek(file, 0L, SEEK_END);
	int length = ftell(file);
	rewind(file);
	
	char *content = (char *) malloc(length);
	int i = 0;
	char c;
	while ((c = fgetc(file)) != EOF)
    {
        content[i] = (char) c;
        i++;
    }
	fclose(file);
	return content;
}

char* get_stdin_content() {
	char *input = NULL;
	int input_size = 0;
	char buffer[1000];
	
	while(fgets(buffer, sizeof(buffer), stdin) != NULL) {
		int new_size = input_size + strlen(buffer);
		char *new_input = (char *) realloc(input, new_size+1);
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

int write_to_output(FILE *file, char* str) {
	if(file == NULL){
		printf("%s", str);
	}else{
		fprintf(file, "%s", str);
	}
	return 0;
}

int main(int argc, char *argv[]){
	int c;
	char *outfilename = NULL;
	FILE *outfile = NULL;
	while((c = getopt(argc,argv,"o:")) != -1){
		switch(c){
			case 'o':
				outfilename = optarg;
				break;
			case '?':
				printf("SYNOPSIS: mycompress [-o outfile] [file...]\n");
				return 1;
		}
	}
	
	int infiles_count = argc - optind;
	int x = argc;
	char *infiles[infiles_count];
	for(int i = optind; i < x; i++) {
		infiles[i - optind] = argv[i];
	}
	
	
	if(outfilename != NULL) {
		outfile = fopen(outfilename, "w");
		if(outfile == NULL) {
			printf("An error occured trying to open %s.\n", outfilename);
			return 1;
		}
	}
		
	char* compressed = NULL;
	if(infiles_count == 0){
		char *input = get_stdin_content();
		compressed = compress(input);
		write_to_output(outfile, compressed);
		free(input);
	}else{
		for(int i = 0; i < infiles_count; i++){
			char *content = get_file_content(infiles[i]);
			char *compressed = compress(content);
			int success = write_to_output(outfile, compressed);
			if(success == 1){
				return 1;
			}
			free(content);
			free(compressed);
		}
	}
	if(outfile != NULL){
		fclose(outfile);
	}
}
