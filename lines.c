#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lines.h" 

// Auxiliary functions.
int recvMessage(int socket, char *buffer, int len) {
    int totalRead = 0;
    int bytesRead;
    
    while (totalRead < len) {
        bytesRead = read(socket, buffer + totalRead, 1); // leer un byte
        
        if (bytesRead <= 0) {
            if (bytesRead == -1 && errno == EINTR)
                continue; // reiniciar la lectura si se interrumpe
            else
                return -1; // error en la lectura
        }
        
        if (buffer[totalRead] == '\0'){
            totalRead++;
            break; // se encontró el carácter nulo, salir del bucle
    }
        totalRead++;

    }
    
    return totalRead;
}

ssize_t readLine(int socket, char *buffer, size_t maxlen) {
    ssize_t numRead = 0;
    char ch = '\0';
    size_t i = 0;

    while ((numRead = recvMessage(socket, &ch, 1)) == 1) {
        if (ch == '\0') {
            buffer[i++] = '\0';
            return i - 1; // return number of bytes read, excluding null terminator
        }

        buffer[i++] = ch;
        if (i == maxlen - 1) {
            buffer[i] = '\0';
            return i;
        }
    }

    return (numRead == 0) ? i : -1;
}

int sendMessage(int socket, char * buffer, int len){

	int r;
	int l = len;
		
	do {	
		r = write(socket, buffer, l); // write to socket
		l = l -r;
		buffer = buffer + r; 
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);   /* fail */
	else
		return(0);	/* full length has been sent */
}
