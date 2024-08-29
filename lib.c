#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define UNEMPLEMENTED \
	do {	\
		fprintf(stderr,"%s:%d: TODO: %s is not implemented yet.\n",\
						__FILE__,__LINE__,__func__);\
		abort();\
		}while(0)


typedef struct
{
	char* buffer;
	size_t buffer_length;
	size_t input_length;
}InputBuffer;


InputBuffer* new_buffer() {
	
	InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	
	input_buffer->buffer = NULL;
        input_buffer->buffer_length = 0;
	input_buffer->input_length = 0;
	
	return input_buffer;
	}

void read_input(InputBuffer* input_buffer) {
	
	ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
	
	if (bytes_read <= 0) {
		printf("Error reading input\n");
		exit(EXIT_FAILURE);
	}
     
	input_buffer->input_length = bytes_read - 1;
	input_buffer->buffer[bytes_read - 1] = 0;
}

void close_buffer(InputBuffer* input_buffer) {
	
	free(input_buffer->buffer);
	free(input_buffer);
}
void input_prompt(){printf("lib >");}

int main()
{
	InputBuffer* input_buffer = new_buffer();
	while(true)
	{
		input_prompt();
		read_input(input_buffer);
		printf("Input : %s",input_buffer->buffer);


	}
}
