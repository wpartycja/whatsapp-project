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
#include "lines.h"

#define MAX_MSG_SIZE 1024

#define DIR_NAME "Database"
#define FILE_TYPE ".txt"
#define VALUE32 32
#define VALUE64 64
#define MAX_SIZE 256

// Desconexion de un cliente
// Solicitud de un musuarios conectados 

// Client register.
int register_client(char *name, char *username, char *birthdate) { 
	int desc;
	int status = 0;
	char temp[100];
	char *line = "Lista de mensajes:\n";
	// Birthdate format DD/MM/AAAA.
	//char ip[32];
	//int port;
	// lista de mensajes pendientes. (?)

	// Verify sizes of values.
	if(strlen(name) > VALUE64){
		printf("Error register_client(): size of name for user %s is bigger than allowed.\n", username);
		printf("----------------------------------------\n");

		return 2;
	}

	if(strlen(username) > VALUE32){
		printf("Error register_client(): size of name for user %s is bigger than allowed.\n", name);
		printf("----------------------------------------\n");

		return 2;
	}

	// Get key as a string and a path to file.
	const char *path = get_path(username);

	// Open the file.
	if((desc = open(path, O_RDONLY)) == -1){
		// Since the file doesnt exist, we proceed to create it.
		if((desc = open(path, O_WRONLY | O_CREAT, 0666)) == -1){
			perror("Error while registering new user.\n");
			printf("----------------------------------------\n");

			return 2;
		}

		char *aux = "\n";
		// Write the information into the file.
		write(desc, username, strlen(username));
		write(desc, aux, strlen(aux));
		write(desc, name, strlen(name));
		write(desc, aux, strlen(aux));
		write(desc, birthdate, strlen(birthdate));
		write(desc, aux, strlen(aux));
		snprintf(temp, 100, "%d", status);
		write(desc, temp, strlen(temp));
		write(desc, aux, strlen(aux));
		write(desc, aux, strlen(aux));
		write(desc, aux, strlen(aux));
		write(desc, line, strlen(line));

		printf("Values for username %s: name =\"%s\", birthdate = %s\n", username, name, birthdate); 
		printf("s> REGISTER <%s> OK\n", username);
	} else {
		// If the file already exists, we cant register the user.
		printf("s> REGISTER <%s> FAIL\n", username);
		printf("----------------------------------------\n");
		return 1;
	}

	// Close the file.
	if(close(desc) == -1){
		perror("Error while closing the file.\n");
		printf("----------------------------------------\n");
		return 2;
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
		printf("s> UNREGISTER <%s> OK\n", username); 
	} else {
		printf("s> UNREGISTER <%s> FAIL\n", username);
		printf("----------------------------------------\n");

		return 1;
	}
	
	printf("----------------------------------------\n");
	return 0;
}

// Client conection.

/* FALTA LO DE LOS MENSAJES
Si una vez conectado, existen mensajes pendientes de enviar para este usuario, 
se enviaran todos los mensajes al usuario uno a uno. 

Si el usuario no existe, se devuelve un 1,  -- OK
si ya est ́a conectado un 2, -- OK
y en cualquier otro caso un 3.

Si existen mensajes pendientes para el usuario que se ha conectado se mostrara el siguiente mensaje por cada 
uno de ellos que se envie:
s> SEND MESSAGE <id> FROM <userNameS> TO <userNameR>
Siendo userNameS el usuario que envio el mensaje originalmente, userNameR el usuario que destinatario 
del mensaje, y id el identificador asociado al mensaje que se envıa. <userNameS> y <userNameR> hacen 
referencia a los alias de los usuarios.*/


// Client connection.
int connect_client(char *username, char *ip, char *port) {
    const char* path = get_path(username);
	char line[MAX_SIZE];
	long int linePosition = -1;
	char *lista = "Lista de mensajes:\n";
	//int lineFound = 0;
	FILE *desc;
    
    // Open the file and check if the user exists.
    desc = fopen(path, "r+");
    if (desc == NULL) {
		// In case the user doesn't exist.
        printf("s> CONNECT <%s> FAIL\n", username);
        printf("----------------------------------------\n");
        return 1; 
    }
    
	// If the user exists, we proceed to check its status.
    while (fgets(line, MAX_SIZE, desc) != NULL) {
        linePosition = ftell(desc) - strlen(line);

        // Check if the line is 0 (disconnected).
		if(strcmp(line, "0\n") == 0){
			// Change status to connected.
			fseek(desc, linePosition, SEEK_SET);
			fwrite("1\n", sizeof(char), strlen("1\n"), desc);
			//lineFound = 1;

			// Add IP.
			fwrite(ip, sizeof(char), strlen(ip), desc);
			fwrite("\n", sizeof(char), strlen("\n"), desc);

			// Add port.
			fwrite(port, sizeof(char), strlen(port), desc);
			fwrite("\n", sizeof(char), strlen("\n"), desc);

			// Write "Lista de mensajes:"
			fwrite(lista, sizeof(char), strlen(lista), desc);
			fwrite("\n", sizeof(char), strlen("\n"), desc);

			fflush(desc);

			printf("s> CONNECT <%s> OK\n", username);
			return 0;
		}

		// Check if the line is 0 (disconnected).
		if(strcmp(line, "1\n") == 0){
			//lineFound = 1;
			printf("s> CONNECT <%s> OK\n", username);
			return 2;
		}
	}
	return 3;
}

// REMEMBER TO CLOSE FILE !!!!!!!!
    /*
    if (status == 0) {
        
        // Send pending messages.
        char send_message[100];
        lseek(desc, -1, SEEK_SET);
        while (lseek(desc, -1, SEEK_CUR) != 0 && read(desc, temp, 1) == 1) {
            if (temp[0] == '\n') {
                break;
            }
        }
        
        if (temp[0] == '1') {
            printf("s> CONNECT <%s> OK\n", username);
            
            // Iterate over pending messages and send them.
            int message_id = 1;
            while (read(desc, send_message, sizeof(send_message)) > 0) {
                printf("s> SEND MESSAGE %d FROM <%s> TO <%s>\n", message_id, get_sender(send_message), username);
                message_id++;
            }
        } else {
            printf("s> CONNECT <%s> FAIL\n", username);
            printf("----------------------------------------\n");
            return 3; // Invalid status value.
        }
    } else if (status == 1) {
        printf("s> CONNECT <%s> FAIL\n", username);
        printf("----------------------------------------\n");
        return 2; // User is already connected.
    } else {
        printf("s> CONNECT <%s> FAIL\n", username);
        printf("----------------------------------------\n");
        return 3; // Invalid status value.
    }
    
    // Close the file.
    if (close(desc) == -1) {
        perror("Error while closing the file.\n");
        printf("----------------------------------------\n");
        return 3;
    }
    
    printf("----------------------------------------\n");
    return 0; // Successful connection.
	*/

// Disconnect a client.
int disconnect_client(char *username) {
	char line[MAX_SIZE];
    long int linePosition = -1;
    FILE *desc;

	// Get path of the file that contains the information.
    const char* path = get_path(username);
  
    // Open the file and check if the user exists.
    desc = fopen(path, "r+");
    if (desc == NULL) {
        // If the user doesn't exist.
        printf("s> DISCONNECT <%s> FAIL\n", username);
        printf("----------------------------------------\n");
        return 1; 
    }
    
    // If the user exists, we proceed to check its status.
    while (fgets(line, MAX_SIZE, desc) != NULL) {
        linePosition = ftell(desc) - strlen(line);

		// Check if the user is not connected.
		if (strcmp(line, "0\n") == 0) {
			printf("s> DISCONNECT <%s> FAIL\n", username);
        	printf("----------------------------------------\n");
			return 2;
		}

        // Check if the line is 1 (connected).
        if (strcmp(line, "1\n") == 0) {
            // Change status to disconnected.
            fseek(desc, linePosition, SEEK_SET);
            fwrite("0\n", sizeof(char), strlen("0\n"), desc);

            // Clear IP and port.
            linePosition = ftell(desc);
            fseek(desc, linePosition, SEEK_SET);


			fwrite("              \n", sizeof(char), strlen("              \n"), desc);
			fwrite("\n", sizeof(char), strlen("\n"), desc);

            fflush(desc);

			if((fclose(desc)) != 0){
				printf("s> DISCONNECT <%s> FAIL\n", username);
    			printf("----------------------------------------\n");
				return 3;
			}

            printf("s> DISCONNECT <%s> OK\n", username);
            printf("----------------------------------------\n");
            return 0;
        }
    }
	if((fclose(desc)) != 0){
		printf("s> DISCONNECT <%s> FAIL\n", username);
    	printf("----------------------------------------\n");
		return 3;
	}
	printf("s> DISCONNECT <%s> FAIL\n", username);
    printf("----------------------------------------\n");
	return 3;
}

// Auxiliary function to verify if user exists and is connected.
int is_connected(char *username){
	// Get path of the user.
	const char* path = get_path(username);

	// Search for the user.
	if (access(path, F_OK) == 0) { // F_OK - test for the existence of the file
		// If user exists, we have to check if it is connected.
		FILE *desc;
		char line[MAX_SIZE];

		desc = fopen(path, "r+");
    	if (desc == NULL) {
        	// Error while opening the file.
        	printf("s> CONNECTEDUSERS FAIL\n");
    		printf("----------------------------------------\n");
			return 3;
    	}

		// Verify if user is connected.
		while (fgets(line, MAX_SIZE, desc) != NULL) {
			// Check if the line is 1 (connected).
			if(strcmp(line, "1\n") == 0){
				// Close the file.
				if((fclose(desc)) != 0){
					printf("s> CONNECTEDUSERS FAIL\n");
    				printf("----------------------------------------\n");
					return 3;
				}
				// User exists and is connected.
				printf("s> CONNECTEDUSERS OK\n");
    			printf("----------------------------------------\n");
				return 0;
			} else if(strcmp(line, "0\n") == 0){
				// User exists but isn't connected.
				printf("User is NOT connected.\n");
				printf("s> CONNECTEDUSERS FAIL\n");
    			printf("----------------------------------------\n");
				return 1;
			}
		}
	} else{
		// If user isn't registered.
		printf("User is NOT registered.\n");
		printf("s> CONNECTEDUSERS FAIL\n");
    	printf("----------------------------------------\n");
		return 2;
	}
	printf("Other error.\n");
	printf("s> CONNECTEDUSERS FAIL\n");
    printf("----------------------------------------\n");
	return 3;
}

// Send client connected users and number of connected users.
int connected_users(int client_sd) {
    DIR *dir;
    struct dirent *entry;
    char line[MAX_SIZE];
    //int count = 0;
    char user[MAX_SIZE];
	char newUser[MAX_SIZE];
    int send;
    char buf[MAX_SIZE];

    // Open the directory.
    dir = opendir(DIR_NAME);
    if (dir == NULL) {
		printf("Error opening directory\n");
        printf("s> CONNECTEDUSERS FAIL\n");
        printf("----------------------------------------\n");
        return 3;
    }

    // Read the files one by one.
    while ((entry = readdir(dir)) != NULL) {
        // Ignore "." and ".." entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char filePath[MAX_SIZE * 2];  // Increase the buffer size

            snprintf(filePath, sizeof(filePath), "%s/%s", DIR_NAME, entry->d_name);

            // Open the user's file.
            FILE *file = fopen(filePath, "r");
            if (file == NULL) {
				printf("Error opening file");
                printf("s> CONNECTEDUSERS FAIL\n");
                printf("----------------------------------------\n");
                return 3;
            }

            // Read each line of the file searching for the status.
            while (fgets(line, sizeof(line), file) != NULL) {
				printf("%s", line);

                // Check if the line equals 0.
                if (strcmp(line, "0\n") == 0) {
                    // This means the user is disconnected.
                    continue;
                }

                // Check if the line equals 1.
                if (strcmp(line, "1\n") == 0) {
					printf("User is connected\n");
                    // This means the user is connected.
                    count++;

                    // Send the username to the client.
                    strncpy(user, entry->d_name, sizeof(user) - 1);
					strncpy(newUser, user, strlen(user) - 4);
                    newUser[sizeof(newUser) - 1] = '\0';

					printf("----- user:\n");
					printf("%s", newUser);
					printf("\n");
					printf("-----\n");

                    send = sendMessage(client_sd, newUser, strlen(newUser));

                    if (send == -1) {
						printf("Error sending user\n");
                        printf("s> CONNECTEDUSERS FAIL\n");
                        printf("----------------------------------------\n");
                        return 3;
                    }
                }
            }

            // Close the file.
            if (fclose(file) != 0) {
                printf("s> CONNECTEDUSERS FAIL\n");
                printf("----------------------------------------\n");
                return 3;
            }
        }
    }

    // Send the counter (number of connected users) to the client.
    //snprintf(buf, sizeof(buf), "%d", count);

    //send = sendMessage(client_sd, buf, strlen(buf));

    //if (send == -1) {
    //    printf("s> CONNECTEDUSERS FAIL\n");
    //    printf("----------------------------------------\n");
    //    return 3;
    //}

    // Close the directory.
    if (closedir(dir) != 0) {
        printf("s> CONNECTEDUSERS FAIL\n");
        printf("----------------------------------------\n");
        return 3;
    }

    return 0;
}

/*

Cuando el servidor recibe la operacio ́n hara ́ lo siguiente:

• Si est ́a conectado, obtendr ́a todos los usuarios conectados en el servicio en ese momento. 
• Enviara ́ al cliente la cantidad de usuarios conectados al servidor. 
• Enviara ́ los usuarios conectados al servicio.

Cuando se realice con  ́exito la obtenci ́on de los usuarios conectados, se mostrar ́a en la consola del servidor el siguiente mensaje:
	s> CONNECTEDUSERS OK
En caso de fallo se mostrara ́:
	s> CONNECTEDUSERS FAIL

0 en caso de  ́exito, 1 si el usuario no est ́a conectado, 2 en cualquier otro caso.
*/

// ----------------------------

// Send message between users.
int send_message(int client_sd, char *username, char *receiver, char *mssg){
	const char* path1 = get_path(username); // Sender.
	const char* path2 = get_path(username);	// Receiver.
	char line[MAX_SIZE];
	char sentence = "Lista de mensajes:\n";

	// First verify if sender exists.
	if (access(path1, F_OK) == 0) { // F_OK - test for the existence of the file
		// If sender exists, we check if receiver exists.
		if (access(path2, F_OK) == 0) { 
			/* Both users exist. We store the message on the queue of the receiver.
			 * Message contains: ID Sender Message.	First we need to retrieve the
			 * ID to know which ID to assign to the new message*/ 
			FILE *file = fopen(path2, "r");
			if (file == NULL) {
                printf("s> SEND FAIL\n");
                printf("----------------------------------------\n");
                return 3;
            }
			// Look for line "Lista de mensajes:"
			while (fgets(line, MAX_SIZE, desc) != NULL) {
				// Check if the line is 1 (connected).
				if(strcmp(line, sentence) == 0){
				}
			}

		}
	} else {
		printf("s> SEND FAIL / USER DOES NOT EXIST\n");
        printf("----------------------------------------\n");
		return 1;
	}

}

/*int send_message(int client_sd, char *username, char *receiver, char *mssg){
	const char* path1 = get_path(username); // Sender.
	const char* path2 = get_path(username);	// Receiver.
	char line[400]; // 400 = ID + Sender + Message.
	char buf[MAX_SIZE];

	// First verify if sender exists.
	if (access(path1, F_OK) == 0) { // F_OK - test for the existence of the file
		// If sender exists, we check if receiver exists.
		if (access(path2, F_OK) == 0) { 
			/* Both users exist. We store the message on the queue of the receiver.
			 * Message contains: ID Sender Message.	First we need to retrieve the
			 * ID to know which ID to assign to the new message*/ /*
			FILE *file = fopen(path2, "r");
			if (file == NULL) {
                printf("s> SEND FAIL\n");
                printf("----------------------------------------\n");
                return 3;
            }

			// Move the file pointer to the last character
			fseek(file, -1, SEEK_END); 

			// We retrieve the first char from the last line.
    		char ch = fgetc(file);
    		while (ch != ' ' && ch != EOF) {
        		fseek(file, -2, SEEK_CUR); 
        		ch = fgetc(file);
   			}

			char firstElement[10];
    		fgets(firstElement, sizeof(firstElement), file);

			if()
			
			// Convert the first element to an integer.
    		int firstElementInt;
    		sscanf(firstElement, "%d", &firstElementInt);

			// We increment the id.
			firstElementInt++;

			// Close the file.
			if (fclose(file) != 0) {
                printf("s> SEND FAIL\n");
                printf("----------------------------------------\n");
                return 3;
            }

			// Now we proceed with writing the message.
			FILE *file2 = fopen(path2, "a");
            if (file2 == NULL) {
                printf("s> SEND FAIL\n");
                printf("----------------------------------------\n");
                return 3;
            }

			// Turn int id into a string.
			char str[MAX_SIZE];
			sprintf(str, "%d", firstElementInt);

			// Give format to the message that will be written.
			strcpy(line, str);
			strcat(line, " ");		
			strcat(line, username);
			strcat(line, " ");
			strcat(line, mssg);

			// Once the file is opened, we store the message at the end.
			fprintf(file2, "%s\n", line); 

			// Close the file.
            if (fclose(file2) != 0) {
                printf("s> SEND FAIL\n");
                printf("----------------------------------------\n");
                return 3;
            }

			// Send results to client and print result message.
			int res = 0;
        	sprintf(buf, "%d", res); // Convert response to string.
        
        	// Send response to client.
        	if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
            	printf("s> SEND FAIL\n");
                printf("----------------------------------------\n");
				return 3;
        	}

			// Send ID of message to client. 
			if((sendMessage(client_sd, firstElement, strlen(firstElement) + 1)) == -1){ 
            	printf("s> SEND FAIL\n");
                printf("----------------------------------------\n");
				return 3;
        	}

			printf("SEND OK - MESSAGE <%d>\n", firstElementInt);
        	printf("----------------------------------------\n");
			return 0;
			 
		} else {
			printf("s> SEND FAIL / USER DOES NOT EXIST\n");
        	printf("----------------------------------------\n");
			return 1;
		}
	} else {
		printf("s> SEND FAIL / USER DOES NOT EXIST\n");
        printf("----------------------------------------\n");
		return 1;
	}
	printf("s> SEND FAIL\n");
    printf("----------------------------------------\n");
    return 3;
}*/

// Deals with the second part of the send message functionality.
