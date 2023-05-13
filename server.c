#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

/*clientes deberan registrarse con un nombre determinado en el sistema y a continuacion conectarse, 
indicando para ello su IP y puerto. El servidor debe mantener una lista con todos los clientes 
registrados, el nombre, estado y direccion de los mismos, ası como una lista de los mensajes 
pendientes de entrega a cada cliente. Ademas se encargara de asociar un identificador a cada 
mensaje recibido de un cliente*/

/*El servidor debe ser capaz de gestionar varias conexiones simultaneamente (debe ser concurrente) 
mediante el uso de multiples hilos (multithread). El servidor utilizara sockets TCP orientados a conexion*/

// Registro de un cliente
// Baja de un cliente
// Conexion de un cliente
// Desconexion de un cliente
// Solicitud de un musuarios conectados 

#define MAX_SIZE    256
#define SLEEP_TIME  4

// mutex & conditions to protect message copy
pthread_mutex_t mutex_connection = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_server = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_message = PTHREAD_COND_INITIALIZER;

int message_not_copied = true; // flag to indicate if message has been copied

// struct to pass arguments to thread
struct client_connection{
    struct sockaddr_in client_addr;
    int client_sd;  
};

// function to deal with client request
void deal_with_message(void *conn){

    // copy message to local variable. 
    pthread_mutex_lock(&mutex_connection);
    struct client_connection *client_conn = (struct client_connection *) conn;
    message_not_copied = false;
    pthread_cond_signal(&cond_message); 
    pthread_mutex_unlock(&mutex_connection);

    // Variables to save values.
    char buf[MAX_SIZE];
    char operation;   
    char name[MAX_SIZE];       
    char username[MAX_SIZE];          
    char birthdate[MAX_SIZE];

    // Unpack values from struct.
    int client_sd = client_conn->client_sd; 

    // Deal with client request and make a response.
    if(recvMessage(client_sd, (char *) &operation, 1) == -1){ 
        fprintf(stderr, "Error: Problem with revceing a message\n");
        close(client_sd); 
        pthread_exit(NULL);
    }

    switch(operation){
	    
	    case 0: // REGISTRO DE UN CLIENTE
		    pthread_mutex_lock(&mutex_server);
            // Call the service.
		    res = init();
		    pthread_mutex_unlock(&mutex_server);
		    break;

	    case 1: // BAJA DE CLIENTE 
		    pthread_mutex_lock(&mutex_server);
            // Get key. 
            res = readLine(client_sd, buf, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Key value could not be received.\n");
                close(client_sd);
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Key value could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            key = atoi(buf); // Convert to int.

            // Get value1.
            res = readLine(client_sd, value1, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server)  value1 could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) value1  could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Get value2.
            res = readLine(client_sd, buf, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server)  value1 could not be received.\n");
                close(client_sd);
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) value1 could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }
            value2 = atoi(buf);

            // Get value3. 
		    res = readLine(client_sd, buf, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server)  value1 could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) value1could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }
            value3 = atoi(buf);

            // Call the service. 
            res = set_value(key, value1, value2, value3);

		    pthread_mutex_unlock(&mutex_server);
		    break;

	     case 2: // CONEXION DE UN CLIENTE 
		     pthread_mutex_lock(&mutex_server);
             // Get key.
             res = readLine(client_sd, buf, MAX_SIZE); 
             if(res == -1){
                // Error receiving message.
                fprintf(stderr, "Error: (Server) Key value could not be received.\n");
                 close(client_sd); 
                 pthread_exit(NULL);
             } else if (res == 0){
                // Error receiving message.
                 fprintf(stderr, "Error: (Server) Key value could not be read.\n");
                 close(client_sd);
                 pthread_exit(NULL);
             }
             key = atoi(buf); // Convert to int.

             // Call the service. 
             res = get_value(key, value1, &value2, &value3);

             // Send result of the operation to the client. 
             sprintf(buf, "%d", res);
             if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
                // Error sending message.
                 printf("Error in communication: Impossible to receive result of the operation\n");
                 close(client_sd);
                 pthread_exit(NULL);
             }

             if(res == 0){
                 // Send value1 to client.
                 if((sendMessage(client_sd, value1, strlen(value1) + 1)) == -1){ 
                    // Error sending message.
                     printf("Error in communication: Impossible to receive value 1\n");
                     close(client_sd);
                     pthread_exit(NULL);
                 }

                 // Send value2 to client.
                 sprintf(buf, "%d", value2);
                 if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
                    // Error sending message.
                     printf("Error in communication: Impossible to receive value 2\n");
                     close(client_sd); 
                     pthread_exit(NULL);
                 }

                 // Send value3 to client.
                 sprintf(buf, "%f", value3);
                 if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
                    // Error sending message.
                     printf("Error in communication: Impossible to receive value 3\n");
                     close(client_sd); 
                     pthread_exit(NULL);
                 }
             }

		     pthread_mutex_unlock(&mutex_server);
		     break;
	    case 3: // DESCONEXION DE UN CLIENTE 
		    pthread_mutex_lock(&mutex_server);
            // Get key.
            res = readLine(client_sd, buf, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Key value could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Key value could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }
            key = atoi(buf);

            // Get value1.
            res = readLine(client_sd, value1, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server)  value1 could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) value1 value could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }

            // Get value2.
            res = readLine(client_sd, buf, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server)  value1 could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) value1 value could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }
            value2 = atoi(buf);

            // Get value3. 
		    res = readLine(client_sd, buf, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server)  value1 could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) value1 value could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }
            value3 = atoi(buf);

            // Call the service. 
            res = modify_value(key, value1, value2, value3);

		    pthread_mutex_unlock(&mutex_server);
		    break;

	    case 4: // SOLICITUD DE USUARIOS CONECTADOS 
		    pthread_mutex_lock(&mutex_server);
            // Get key.
            res = readLine(client_sd, buf, MAX_SIZE); 
            if(res == -1){
                fprintf(stderr, "Error: (Server) Key value could not be received.\n");
                close(client_sd); 
                pthread_exit(NULL);
            } else if (res == 0){
                fprintf(stderr, "Error: (Server) Key value could not be read.\n");
                close(client_sd); 
                pthread_exit(NULL);
            }
            key = atoi(buf);

            // Call the service. 
            res = delete_key(key);

		    pthread_mutex_unlock(&mutex_server);
		    break;
		    
        default:
            break;

    }     

    // Send response to client.
    if(operation != 2){
        sprintf(buf, "%d", res); // Convert response to string.
        // Send response to client.
        if((sendMessage(client_sd, buf, strlen(buf) + 1)) == -1){ 
            fprintf(stderr, "Error: (Server) response value could not be sent to the client.");
            close(client_sd); 
            pthread_exit(NULL);
        }
    }
    
    // Close the connection with the client.
    printf("Closing connection with client.\n");
    close(client_sd); 
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_attr_t thread_attr; // threads attributes
    pthread_t thid; // thread id
    
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

    struct client_connection client_conn; // Client connection structure.
    socklen_t client_address_len = sizeof(client_conn.client_addr); // Client address length.

    // Create socket.
    int socket_fd = socket(
            AF_INET,
            SOCK_STREAM,
            0
    );

    // Check if socket was created correctly.
    if (socket_fd < 0) {
        perror("error creating socket");
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
        perror("error binding socket");
        return 1;
    }

    // Put socket in listening mode.
    int listen_result = listen(socket_fd, 1);
    if (listen_result < 0) {
        perror("error putting socket in listening mode");
        return 1;
    }

    // Initialize mutex and condition variables.
    pthread_mutex_init(&mutex_connection, NULL); 
    pthread_cond_init(&cond_message, NULL); 
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED); // Independent threads.

    // Listen for connections.
    while (1) {

        // Accept connection.
        client_conn.client_sd = accept(socket_fd, (struct sockaddr*) &client_conn.client_addr, &client_address_len);
        if (client_conn.client_sd < 0) {
            perror("error accepting connection");
            return 1;
        } else {
            printf("Connection succesfully accepted from IP: %s\tPort: %d\n", inet_ntoa(client_conn.client_addr.sin_addr), ntohs(client_conn.client_addr.sin_port));
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
