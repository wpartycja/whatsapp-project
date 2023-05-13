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
#define MAX_SIZE 256

#define DIR_NAME "Database"
#define FILE_TYPE ".txt"
#define VALUE32 32
#define VALUE64 64


// Conexion de un cliente
// Desconexion de un cliente
// Solicitud de un musuarios conectados 

// Client register.
int register_client(char *name, char *username, char *birthdate) { 
	int desc;
	int status = 0;
	char temp[100];
	// Birthdate format DD/MM/AAAA.
	//char ip[32];
	//int port;
	// lista de mensajes pendientes. (?)

	char line[5000];
	
	int n;

	// Verify sizes of values.
	if(strlen(name) > VALUE64){
		printf("Error register_client(): size of name for user %s is bigger than allowed.\n", username);
		printf("----------------------------------------\n");

		return -1;
	}

	if(strlen(username) > VALUE32){
		printf("Error register_client(): size of name for user %s is bigger than allowed.\n", username);
		printf("----------------------------------------\n");

		return -1;
	}

	// Get key as a string and a path to file.
	const char *path = get_path(username);

	// Open the file.
	if((desc = open(path, O_RDONLY)) == -1){
		// Since the file doesnt exist, we proceed to create it.
		if((desc = open(path, O_WRONLY | O_CREAT, 0666)) == -1){
			perror("Error while registering new user.\n");
			printf("----------------------------------------\n");

			return -1;
		}
		// Write the information into the file.
		write(desc, username, strlen(username));
		write(desc, name, strlen(name));
		write(desc, birthdate, strlen(birthdate));
		snprintf(temp, 100, "%d", status);
		write(desc, temp, strlen(temp));

		printf("Values for username %s: name =\"%s\", birthdate = %s\n", username, name, birthdate); 
		printf("REGISTER <%s> OK\n", username);
	} else {
		// If the file already exists, we cant register the user.
		printf("REGISTER <%s> FAIL\n", username);
		printf("----------------------------------------\n");
		return -1;
	}

	// Close the file.
	if(close(desc) == -1){
		perror("Error while closing the file.\n");
		printf("----------------------------------------\n");
		return -1;
	}

	printf("----------------------------------------\n");
	return 0;

}

// Delete a client.
int unregister_client(char *username){
	// Get the path of the file.
	const char* path = get_path(username); 

	// Try to delete the file.
	if(remove(path) == 0) {
		printf("UNREGISTER <%s> OK\n", username); 
	} else {
		printf("UNREGISTER <%s> FAIL\n", username);
		printf("----------------------------------------\n");

		return -1;
	}
	
	printf("----------------------------------------\n");
	return 0;
}
