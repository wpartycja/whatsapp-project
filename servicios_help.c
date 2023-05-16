#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "servicios_help.h"

#define DIR_NAME "Database"
#define FILE_TYPE ".txt"
#define MAX_SIZE 256

// Function to get the path of the file.
const char* get_path(const char* username){
	// creating a path to file
	char path[strlen(DIR_NAME) + strlen(username) + strlen(FILE_TYPE) + 2]; // 2 becuase of "/" and EOF
	snprintf(path, sizeof(path), "%s/%s%s", DIR_NAME, username, FILE_TYPE);

	// creating a pointer to the path of the file
	char* path_ptr = (char*)malloc(strlen(DIR_NAME) + strlen(username) + strlen(FILE_TYPE) + 2);
	strcpy(path_ptr, path);

	return path_ptr;
}

void clean_char_array(char arr[], int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = '\0';
    }
}