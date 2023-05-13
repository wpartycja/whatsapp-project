#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>

#include "servicios.h"
#include "servicios_help.h"

#define MAX_MSG_SIZE 1024
#define FILE_TYPE ".txt"
#define MAX_SIZE 256

#define DIR_NAME "Database"
#define VALUE32 32;
#define VALUE64 64;

// Registro de un cliente
// Baja de un cliente
// Conexion de un cliente
// Desconexion de un cliente
// Solicitud de un musuarios conectados 

// Client register.
int register_client(char name, char username, char birthdate) { 
	char name[64];
	char username[32];
	//formato DD/MM/AAAA.
	char birthdate[32];
	int status = 0;
	//char ip[32];
	//int port;
	// lista de mensajes pendientes. (?)

	char line[5000];
	char temp[1000];
	int n;

	// Verify sizes of values.
	if(strlen(name) > VALUE64){
		printf("Error register_client(): size of name for user %d is bigger than allowed.\n", username);
		printf("----------------------------------------\n");

		return -1;
	}

	if(strlen(username) > VALUE32){
		printf("Error register_client(): size of name for user %d is bigger than allowed.\n", username);
		printf("----------------------------------------\n");

		return -1;
	}

	// Get key as a string and a path to file.
	const char *userStr = get_username_str(username);
	const char *path = get_path(userStr);

	// Open the file.
	if((status = open(path, O_RDONLY)) == -1){
		// Since the file doesnt exist, we proceed to create it.
		if((status = open(path, O_WRONLY | O_CREAT, 0666)) == -1){
			perror("Error while creating the file.\n");
			printf("----------------------------------------\n");

			return -1;
		}
		// Format the key and values to write them into the file.
		snprintf(line, 5000, "%d, ", key);
		
		n = strlen(value1);
		strncat(line, value1, n);
		
		snprintf(temp, 1000, ", %d, ", value2); 
		n = strlen(temp);
		strncat(line, temp, n);
		
		snprintf(temp, 1000, "%lf\n", value3);
		n = strlen(temp);
		strncat(line, temp, n);

		// Write the values into the file.
		write(status, line, strlen(line));

		printf("Values for key %d: value1 =\"%s\", value2 = %d, value3 = %lf\n", key, value1, value2, value3);
		printf("Successfully set values for key %d\n", key);
	} else {
		// Since file already exists, it is considered an error.
		printf("Error set_value(): key %d value already exists.\n", key);
		printf("----------------------------------------\n");
		return -1;
	}

	// Close the file.
	if(close(status) == -1){
		perror("Error while closing the file.\n");
		printf("----------------------------------------\n");
		return -1;
	}

	printf("----------------------------------------\n");
	return 0;
}

int unregister_client(int key){
	// add variable key in char[] type
	const char* key_str = get_key_str(key);
	const char* path = get_path(key_str);

	// try to delete the file
	if(remove(path) == 0) {
		printf("File %s%s deleted successfully!\n", key_str, FILE_TYPE);
	} else {
		printf("Error delete_key(): Unable to delete file %s%s\n", key_str, FILE_TYPE);
		printf("----------------------------------------\n");

		return -1;
	}
	
	printf("----------------------------------------\n");
	return 0;
}
