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
	
	if (input_buffer->buffer == NULL) {
		input_buffer->buffer = (char*)malloc(1024);
		input_buffer->buffer_length = 1024;
	}

	if (fgets(input_buffer->buffer, input_buffer->buffer_length, stdin) == NULL) {
		printf("Error reading input\n");
		exit(EXIT_FAILURE);
	}
	
	input_buffer->input_length = strlen(input_buffer->buffer);
	if (input_buffer->buffer[input_buffer->input_length - 1] == '\n') {
		input_buffer->buffer[input_buffer->input_length - 1] = 0;
		input_buffer->input_length--;
	}
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

	}
}
