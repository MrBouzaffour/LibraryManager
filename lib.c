#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define BUFFERCAP 1024
#define UNEMPLEMENTED \
	do {	\
		fprintf(stderr,"%s:%d: TODO: %s is not implemented yet.\n",\
						__FILE__,__LINE__,__func__);\
		abort();\
		}while(0)


typedef struct {
	char* buffer;
	size_t buffer_length;
	size_t input_length;
}InputBuffer;


typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED
}MetaCommandResult;

typedef enum {
	PREPARE_SUCCESS,
	PREPARE_UNRECOGNIZED,
	PREPARE_ERROR
} PrepareResult;

typedef enum {
	INSERT,
	SELECT
}StatementType;

typedef struct {
	StatementType type;
}Statement;

typedef enum {
	SUCCESS,
	FAILED,
	NOT_FOUND
} ExecutedResult;

InputBuffer* new_buffer() {
	
	InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	
	input_buffer->buffer = NULL;
        input_buffer->buffer_length = 0;
	input_buffer->input_length = 0;
	
	return input_buffer;
}

void read_input(InputBuffer* input_buffer) {
	
	if (input_buffer->buffer == NULL) {
		input_buffer->buffer = (char*)malloc(BUFFERCAP);
		input_buffer->buffer_length = BUFFERCAP;
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

MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
	
	if (strcmp(input_buffer->buffer, ".exit") == 0) {
		exit(EXIT_SUCCESS);
	} else{
		return META_COMMAND_UNRECOGNIZED;
	}
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
	(void)input_buffer;
	(void)statement;
	UNEMPLEMENTED;
}


int main()	
{
	InputBuffer* input_buffer = new_buffer();
	while(true)
	{
		input_prompt();
		read_input(input_buffer);
		 if (input_buffer->buffer[0] == '.'){
			 switch( do_meta_command(input_buffer)){
				 case(META_COMMAND_SUCCESS):
					 continue;
				 case(META_COMMAND_UNRECOGNIZED):
					 printf("Unrecognized command '%s'\n", input_buffer->buffer);
					 continue;
			 }
		 }
		Statement statement;
		switch ( prepare_statement(input_buffer, &statement)) {
			
			case(PREPARE_SUCCESS):
				break;
			case (PREPARE_ERROR):
				printf("Syntax error. Could not parse statement.\n");
				continue;
			case(PREPARE_UNRECOGNIZED):
				printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
				continue;
		}
	}
}
