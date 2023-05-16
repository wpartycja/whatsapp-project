#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lines.h"
#include "servicios.h"

#define DIR_NAME "Database"
#define FILE_TYPE ".txt"

#define MAX_SIZE    256 

int connectedUsers = 0;

// Mutex & conditions to protect message copy.
pthread_mutex_t mutex_connection = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_server = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_message = PTHREAD_COND_INITIALIZER;

// Flag to indicate if message has been copied.
int message_not_copied = true; 

// Struct to pass arguments to thread.
struct client_connection{
    struct sockaddr_in client_addr;
    int client_sd;  
};

int init() {
	DIR* dir = opendir(DIR_NAME); // Open the directory.

	// Search for the directory.
	if(dir) {
		// Close the directory.
		if((closedir(dir)) == -1){
			perror("Error init(): while closing the directory.\n");
			printf("----------------------------------------\n");

			return 2;
		}

	} else if (ENOENT == errno){
		// If the directory doesnt exist, we create it. 
		mkdir(DIR_NAME, 0700);
	} else {
		perror("Error init(): while creating the database.\n");
		printf("----------------------------------------\n");

		return 2;
	}

	printf("Initialization completed successfully.\n");
	printf("----------------------------------------\n");

	return 0;
}

// Function to deal with client request.
void deal_with_message(void *conn){
    // copy message to local variable. 
    pthread_mutex_lock(&mutex_connection);
    struct client_connection *client_conn = (struct client_connection *) conn;
    message_not_copied = false;
    char ip[MAX_SIZE];
    strcpy(ip, inet_ntoa(client_conn->client_addr.sin_addr));
    pthread_cond_signal(&cond_message); 
    pthread_mutex_unlock(&mutex_connection);

    // Variables to save values.
    char operation[MAX_SIZE];  
    int number;
    char buf[MAX_SIZE];
    int res;
    char name[MAX_SIZE];
	char username[MAX_SIZE];   
	char birthdate[MAX_SIZE];   //Format DD/MM/AAAA.
    char port[MAX_SIZE];
    char mssg[MAX_SIZE];
    char receiver[MAX_SIZE];

    // Unpack values from struct.
    int client_sd = client_conn->client_sd; 

    // Deal with client request and make a response.
    if(recvMessage(client_sd, (char *) &operation, 256) == -1){ 
        fprintf(stderr, "Error: Problem with receiving a message.\n");
        close(client_sd); 
        pthread_exit(NULL);
    }

    // Converting service string to its int equivalent.
    if (strcmp(operation, "REGISTER") == 0) {
        number = 0;
    } else if (strcmp(operation, "UNREGISTER") == 0) {
        number = 1;
    } else if (strcmp(operation, "CONNECT") == 0) {
        number = 2;
    } else if (strcmp(operation, "DISCONNECT") == 0) {
        number = 3;
    } else if (strcmp(operation, "SEND") == 0) {
        number = 4;
    } else if (strcmp(operation, "CONNECTEDUSERS") == 0) {
        number = 5;
    } 

    // Services.
    switch(number){
        // REGISTER CLIENT.
	    case 0: 
		    pthread_mutex_lock(&mutex_server);
            // Get name. 
            res = readLine(client_sd, name, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) name could not be received.\n");
                close(client_sd);
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) name could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Get username.
            res = readLine(client_sd, username, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) username could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) username could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Get date.
            res = readLine(client_sd, birthdate, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) birthdate could not be received.\n");
                close(client_sd);
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) birthdate could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Call the service. 
            res = register_client(name, username, birthdate);

		    pthread_mutex_unlock(&mutex_server);
		    break;

        // UNREGISTER CLIENT
	    case 1: 
            pthread_mutex_lock(&mutex_server);
            // Get username.
            res = readLine(client_sd, username, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) username could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) username could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Call the service. 
            res = unregister_client(username);

		    pthread_mutex_unlock(&mutex_server);
		    break;

        // CONNECT CLIENT.
        case 2:
            pthread_mutex_lock(&mutex_server);
            // Get username.
            res = readLine(client_sd, username, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Username could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Username could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Get port.
            res = readLine(client_sd, port, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Port could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Port could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Call the service. 
            res = connect_client(username, ip, port);

            if(res == 0){
                connectedUsers++;
            }

		    pthread_mutex_unlock(&mutex_server);
            break;
        
        // DISCONNECT CLIENT
        case 3:
            pthread_mutex_lock(&mutex_server);
            printf("Start case 3 - DISCONNECT\n");

            // Get username.
            res = readLine(client_sd, username, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Username could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Username could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Call the service. 
            res = disconnect_client(username);

            if(res == 0){
                connectedUsers--;
            }

		    pthread_mutex_unlock(&mutex_server);
            break;

        // SEND.
        case 4:
            pthread_mutex_lock(&mutex_server);
            // Get username that sends the mesage.
            res = readLine(client_sd, username, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Username of sender could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Username of sender could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }
            printf("Sender: %s\n", username);

            // Get username that will receive the message.
            res = readLine(client_sd, receiver, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Username of receiver could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Username of sender could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Get message.
            res = readLine(client_sd, mssg, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Message could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Message could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Call service.
            res = send_message(client_sd, username, receiver, mssg);

            pthread_mutex_unlock(&mutex_server);
            break;

        // CONNECTED USERS.
        case 5:
            pthread_mutex_lock(&mutex_server);
            printf("Start case 4 - CONNECTED USERS\n");

            // Get username.
            res = readLine(client_sd, username, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Username could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Username could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Send number of connected users to client.
            sprintf(buf, "%d", connectedUsers); // Convert response to string.

            if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
                fprintf(stderr, "Error: (Server) response value could not be sent to the client.");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Call the first service.
            res = is_connected(username);

            // Send response to client.
            sprintf(buf, "%d", res); // Convert response to string.

            // Send response to client.
            if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
                fprintf(stderr, "Error: (Server) response value could not be sent to the client.");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Verify if second service should be called. 
            if(res == 0){
                printf("\n");
                res = connected_users(client_sd);

                // Send response to client.
                sprintf(buf, "%d", res); // Convert response to string.
        
                // Send response to client.
                if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
                    fprintf(stderr, "Error: (Server) response value could not be sent to the client.");
                    close(client_sd); 
                    pthread_exit(NULL);
                }
            }

            pthread_mutex_unlock(&mutex_server);
            break;
        
        default:
            break;
    }     

    if(number != 5){
        // Send response to client.
        sprintf(buf, "%d", res); // Convert response to string.
        
        // Send response to client.
        if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
            fprintf(stderr, "Error: (Server) response value could not be sent to the client.");
            close(client_sd); 
            pthread_exit(NULL);
        }
    }

    // Close the connection with the client.
    //printf("Closing connection with client.\n");
    close(client_sd); 
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_attr_t thread_attr; // Threads attributes.
    pthread_t thid; // Thread id.

    // El servidor se ejecutara de la siguiente manera: $ ./server -p <port>  codigo
    //Al iniciar el servidor se mostrara el siguiente mensaje: s> init server <localIP>:<port>
    //Antes de recibir comandos por parte de los clientes mostrara ́: s>
    //El programa terminara al recibir una sen ̃al SIGINT (Ctrl+C). 
    
    // Check if the number of arguments is correct.
    if (argc != 2){
        printf("Wrong number of arguments, system requires port number!\n");
        return 1;
    }

    int port = atoi(argv[1]); // Get port number.

    // Create needed structures.
    struct sockaddr_in server_address = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = htonl(INADDR_ANY),
            .sin_port = htons(port)
    };

    // Client connection structure.
    struct client_connection client_conn; 

    // Client address length.
    socklen_t client_address_len = sizeof(client_conn.client_addr); 

    // Create socket.
    int socket_fd = socket(
            AF_INET,
            SOCK_STREAM,
            0
    );

    // Check if socket was created correctly.
    if (socket_fd < 0) {
        perror("Error creating socket.\n");
        return 1;
    }

    // Bind socket to address.
    int bind_result = bind(
            socket_fd,
            (const struct sockaddr *) &server_address,
            sizeof(server_address)
    );

    // Check if socket was binded correctly.
    if (bind_result < 0) {
        perror("Error binding socket.\n");
        return 1;
    }

    // Put socket in listening mode.
    int listen_result = listen(socket_fd, 1);
    if (listen_result < 0) {
        perror("Error putting socket in listening mode.\n");
        return 1;
    }

    // Initialize mutex and condition variables.
    pthread_mutex_init(&mutex_connection, NULL); 
    pthread_cond_init(&cond_message, NULL); 
    pthread_attr_init(&thread_attr);
    
    // Independent threads.
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED); 

    init();

    // Listen for connections.
    while (1) {

        // Accept connection.
        client_conn.client_sd = accept(socket_fd, (struct sockaddr*) &client_conn.client_addr, &client_address_len);
        if (client_conn.client_sd < 0) {
            perror("Error accepting connection.\n");
            return 1;
        } else {
            printf("init server <localIP: %s>:<port: %d>\n", inet_ntoa(client_conn.client_addr.sin_addr), ntohs(client_conn.client_addr.sin_port));
        }

        if (pthread_create(&thid, &thread_attr, (void *) deal_with_message, &client_conn) == 0) {
            // Wait until thread copy message.
            pthread_mutex_lock(&mutex_connection);
            while (message_not_copied)
                pthread_cond_wait(&cond_message, &mutex_connection); 
            message_not_copied = true;
            pthread_mutex_unlock(&mutex_connection);
        }
    }

    pthread_mutex_destroy(&mutex_connection);
    pthread_mutex_destroy(&mutex_server);
    pthread_cond_destroy(&cond_message);
    pthread_attr_destroy(&thread_attr);

    return 0;
} 