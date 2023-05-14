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
        
        if (buffer[totalRead] == '\0')
            break; // se encontró el carácter nulo, salir del bucle
        
        totalRead++;
    }
    
    return totalRead;
}

// ---------------------------------------
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
// ---------------------------------------

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

/*
ssize_t readLine(int fd, void *buffer, size_t n){
	ssize_t numRead;  // num of bytes fetched by last read() 
	size_t totRead;	  // total bytes read so far 
	char *buf;
	char ch;
	int flag = 0;

	if (n <= 0 || buffer == NULL) { 
		errno = EINVAL;
		return -1; 
	}

	buf = buffer;
	totRead = 0;
	
	for (;;) {
		//printf("Entra for\n");
		numRead = read(fd, &ch, 1);	// read a byte 
		//printf("%ld\n", numRead);

		if (ch == '\0'){
			if(flag == 0){
				printf("es 0 \n");
				numRead = read(fd, &ch, 1);
				flag = 1;
			}
		}

    	if (numRead == -1) {
			printf("numread -1\n");	
            if (errno == EINTR)	// interrupted -> restart read() 
        		continue;
        	else
				return -1;		// some other error 
		} else if (numRead == 0) {	// EOF 
			printf("numread 0\n");
        	if (totRead == 0)	// no byres read; return 0 
           		return 0;
			else
          		break;
        } else {			// numRead must be 1 if we get here
        	if (ch == '\n')
           		break;
        	if (ch == '\0'){
				//printf("Es barra 0\n");
          		break;
			}
            if (totRead < n - 1) {		// discard > (n-1) bytes 
				totRead++;
				*buf++ = ch; 
			}
		} 
	}
	
	*buf = '\0';
    
	return totRead;
} */
