#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define BUFFERCAP 1024
#define BOOKCAP 1024
#define LIBRARIESCAP  1024

#define UNIMPLEMENTED \
	do {	\
		fprintf(stderr,"%s:%d: TODO: %s is not implemented yet.\n",\
						__FILE__,__LINE__,__func__);\
		abort();\
		}while(0)


typedef struct {
	int day;
	int month;
	int year;	
} Date;

typedef struct {
	char Name[10];
	char LastName[10];
	Date DateBirth;
} Author;

typedef struct {
	char Name[20];
	char Genre[20];
	Author Auth;
} Book;

typedef struct {
	char* name;
	char* location;
	Book books[BOOKCAP];
} Library;

typedef struct {
	Library* libraries;
	int count;
} Libraries;

typedef struct {
	char* buffer;
	size_t buffer_length;
	size_t input_length;
} InputBuffer;

typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED
} MetaCommandResult;

typedef enum {
	PREPARE_SUCCESS,
	PREPARE_UNRECOGNIZED,
	PREPARE_ERROR
} PrepareResult;

typedef enum {
 SHOW,
	INSERT,
	SELECT
} StatementType;

typedef struct {
	StatementType type;
} Statement;

typedef enum {
	SUCCESS,
	FAILED,
	NOT_FOUND
} ExecutedResult;


Libraries init_Libraries() {
	Libraries libs;
	libs.libraries = malloc( LIBRARIESCAP * sizeof(Libraries) );
	libs.count = 0;
	
	if(libs.libraries == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Step 1 : INIT LIBRARIES\n");
	return libs;
}


Library init_Library(char *name, char *location) {
	Library lib;

	lib.name = malloc(strlen(name)*sizeof(char));
	lib.location = malloc(strlen(location)*sizeof(char));
	if (lib.name == NULL || lib.location == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
	
	strcpy(lib.name,name);
	strcpy(lib.location,location);
	memset(lib.books,0,sizeof(lib.books));
	return lib;
}

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

void input_prompt(){printf(">> ");}

MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
	if (strcmp(input_buffer->buffer, ".exit") == 0) {
		exit(EXIT_SUCCESS);
	} else{
		return META_COMMAND_UNRECOGNIZED;
	}
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement,Libraries* libs, Library* lib) {
	 if (strncmp(input_buffer->buffer, "show", 4) == 0) {
        statement->type = SHOW;
								if (libs.count == 0) {
											printf("thete is no libraries registered. /n");
										 
								}
								for (int i = 0; i <= libs.count; ++i) {
												if (libs.libraries[i].name == lib.name ) {
														printf("*  %s./n",lib.name);
														}
													else {
														printf("%s",libs.libraries[i].name);
													}
									}
        
     return PREPARE_SUCCESS;
        
        
        }
	(void)input_buffer;
	(void)statement;
	UNIMPLEMENTED;
}

ExecutedResult execute_statement(Statement* statement) {
	(void) statement;
	UNIMPLEMENTED;
/*
 	switch (statement->type) {
	    case (STATEMENT_INSERT):
		return execute_insert(statement, table);
	case (STATEMENT_SELECT_ALL):
		return execute_select_all(statement, table);
	case (STATEMENT_SELECT):
	return execute_select(statement, table);
	}*/
}

int main()	
{
	InputBuffer* input_buffer = new_buffer();
	Libraries libs = init_Libraries();
	printf("there are %d libraries \n",libs.count);
	while(true)
	{
		input_prompt();
		read_input(input_buffer);
		if (input_buffer->buffer[0] == '.') {
			switch( do_meta_command(input_buffer)) {
				case(META_COMMAND_SUCCESS):
					continue;
				case(META_COMMAND_UNRECOGNIZED):
					printf("Unrecognized command '%s'\n", input_buffer->buffer);
					continue;
			}
		}
		
		Statement statement;
		switch (prepare_statement(input_buffer, &statement)) {	
			case(PREPARE_SUCCESS):
				break;
			case (PREPARE_ERROR):
				printf("Syntax error. Could not parse statement.\n");
				continue;
			case(PREPARE_UNRECOGNIZED):
				printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
				continue;
		}
		
		switch (execute_statement(&statement)) {
			case (SUCCESS):
				printf("Executed.\n");
				break;
			case (FAILED):
				printf("Error: Execution failed.\n");
				break;
			case (NOT_FOUND):
				printf("Error: Data Not found.\n");
		}
	}
}
