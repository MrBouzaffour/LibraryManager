#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define BUFFERCAP 1024	// Input Size 
#define BOOKCAP 1024	// capacity of books in a libraries
#define LIBRARIESCAP  1024	// Maximum number of libraries

/* This macro will help me in debugging. Remove later :) */
#define UNIMPLEMENTED \
	do {	\
		fprintf(stderr,"%s:%d: TODO: %s is not implemented yet.\n",\
						__FILE__,__LINE__,__func__);\
		abort();\
		}while(0)



/* -- Date : year/month/day -- */
typedef struct {
	int day;
	int month;
	int year;	
} Date;

/* The Author of the book's informations */
typedef struct {
	char* Name;
	char* LastName;
	Date DateBirth;
} Author;

/* The book's informations */
typedef struct {
	char Name[20];
	char Genre[20];
	Author Auth;
} Book;

/* -- Local library informations && the book list that it contains -- */
typedef struct {
	char* name;
	//char* location;
	Book books[BOOKCAP];
} Library;

/* 
	This will represent all libraries: list of libraries, how many libraries,
	and a pointer to the current library that we are using
 */
typedef struct {
	Library* libraries;
	int count;
	Library* currentlib;
} Libraries;


/* --Input - > statement - > prepare - > execute */

/* Input */
typedef struct {
	char* buffer;
	size_t buffer_length;
	size_t input_length;
} InputBuffer;

/* statement prepare */
typedef struct {
	StatementType type;
	Library* auxlib;
} Statement;

/* statement Type */
typedef enum {
 	SHOW,
	USE
	//INSERT,
	//SELECT
} StatementType;

/* Meta commands */
typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED
} MetaCommandResult;

/* Prepare Results*/
typedef enum {
	PREPARE_SUCCESS,
	PREPARE_UNRECOGNIZED,
	PREPARE_ERROR
} PrepareResult;

/* Executed Results */
typedef enum {
	SUCCESS,
	FAILED,
	NOT_FOUND
} ExecutedResult;

/* Dubugging Results */
typedef enum {
	DEBUGGING_SUCCESS,
	DEBUGGING_FAILLED
} DebuggingResult;

/* Exit */
typedef enum {
	EXIST,
	NOTEXIST
}Exist;



Libraries libs;

void input_prompt(){printf(">> ");}

void init_Libraries(Libraries* libs) 
{
	/*
	*	Memory allocation for the libraries buffer
	*/
	libs->libraries = malloc(LIBRARIESCAP * sizeof(Library));
	libs->count = 0;
	
	if(libs->libraries == NULL) {
		fprintf(stderr, "allocation failed\n");
		exit(EXIT_FAILURE);
	}
}

InputBuffer* new_buffer() 
{	
	/*
	*	Memory allocation for the input buffer
	*/
	InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	
	input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
	input_buffer->input_length = 0;
	
	return input_buffer;
}

void read_input(InputBuffer* input_buffer) 
{
	/*
	*	Reading input and storing it into the input buffer
	*/
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

void close_buffer(InputBuffer* input_buffer) 
{
	/*
	*	Closing input buffer
	*/	
	free(input_buffer->buffer);
	free(input_buffer);
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer) 	
{
	/*
	*	Checking if the input is recognized as a META_COMMAND like : '.exit'
	*/
	if (strcmp(input_buffer->buffer, ".exit") == 0) {
		exit(EXIT_SUCCESS);
	} else{
		return META_COMMAND_UNRECOGNIZED;
	}
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement, Libraries* libs) 
{
	/*
	*	This functions figures out what type of statement we are getting, prepare it and debug it 
	*/	
	(void)libs; 

	//	>> SHOW
	if (strncmp(input_buffer->buffer, "show", 4) == 0) {
		
		statement->type = SHOW;
		
		switch (debug(input_buffer, statement, libs)) {
			case (DEBUGGING_SUCCESS):
				return PREPARE_SUCCESS;
			case (DEBUGGING_FAILLED):
				return PREPARE_UNRECOGNIZED;	
		}
	}

	//	>> USE lib1
	if (strncmp(input_buffer->buffer, "use", 3) == 0) {
		
		statement->type = USE;
		Library* new_lib = malloc(sizeof(Library));
		new_lib->name = malloc(20 * sizeof(char));
		
		int args_assigned = sscanf(input_buffer->buffer,"use %19s",new_lib->name);
		
		if (args_assigned == 1) {
			statement->auxlib = new_lib;
			return PREPARE_SUCCESS;
		} else {
			free(new_lib);
			return PREPARE_UNRECOGNIZED;
		}
	}
	return PREPARE_UNRECOGNIZED;
}

DebuggingResult debug (InputBuffer* input_buffer, Statement* statement, Libraries* libs) 
{
	/*
	*	Debug a statement
	*/
	(void)libs; 

	switch(statement->type) {
		case (SHOW):
			if (strlen(input_buffer->buffer) != 4) {
				printf("Error: 'show' command should be exactly 4 characters long.\n");
				return DEBUGGING_FAILLED;}
			break;
			
		default:
			return DEBUGGING_SUCCESS;
	}
	return DEBUGGING_SUCCESS;
}

ExecutedResult execute_statement(Statement* statement, Libraries* libs) 
{
	/*
	*	Executing the statement based on its type
	*/
	switch (statement->type) {
		/*case (INSERT):
			return execute_insert(statement);
		case (SELECT):
			return execute_select(statement);*/
		case (SHOW):
			return execute_show(libs);
		case (USE):
			return execute_use(statement,libs);
		default:
			return FAILED;
	}
}


ExecutedResult execute_show(Libraries* libs) {
	printf("\n There is %d libraries in totak.\n\n",libs->count);
	if (libs->count == 0) {
		printf("\n There is no libraries registered yet. \n");
	}
	else {
		for (int i = 0; i < libs->count; ++i) {
			if (strcmp(libs->libraries[i].name, libs->currentlib->name) == 0) {
    			printf("*  %s.\n",libs->currentlib->name);
			} 
			else {
				printf("%s. \n",libs->libraries[i].name);
			}
		}
	}
	return SUCCESS;
}





ExecutedResult execute_use(Statement* statement, Libraries* libs) {
    if (statement->auxlib != NULL) {
        switch (libExist(libs, statement->auxlib)) {
            case EXIST:
                // If library exists, set it as current library
                for (int i = 0; i < libs->count; ++i) {
                    if (strcmp(libs->libraries[i].name, statement->auxlib->name) == 0) {
                        libs->currentlib = &libs->libraries[i];
                        return SUCCESS;
                    }
                }
                break;

            case NOTEXIST:
                // If the library doesn't exist, add it to the libraries array
                if (libs->count < LIBRARIESCAP) { // Ensure we have space for new library
                    // Copy the new library to the array of libraries
                    libs->libraries[libs->count] = *statement->auxlib;
                    libs->count++;
                    libs->currentlib = &libs->libraries[libs->count - 1]; // Set as current library
                    return SUCCESS;
                } else {
                    printf("Error: Maximum number of libraries reached.\n");
                    return FAILED;
                }
        }
    }
    return FAILED;
}

Exist libExist(Libraries* libs, Library* lib) {
    for (int i = 0; i < libs->count; ++i) {
        if (strcmp(libs->libraries[i].name, lib->name) == 0) {
            return EXIST;
        }
    }
    return NOTEXIST;
}



int main()	
{
	InputBuffer* input_buffer = new_buffer();
	init_Libraries(&libs);
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
		switch (prepare_statement(input_buffer, &statement, &libs)) {	
			case(PREPARE_SUCCESS):
				break;
			case (PREPARE_ERROR):
				printf("Syntax error. Could not parse statement.\n");
				continue;
			case(PREPARE_UNRECOGNIZED):
				printf("Unrecognized keyword at start of '%s'.\n",input_buffer->buffer);
				continue;
		}
		
		switch (execute_statement(&statement, &libs)) {
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
