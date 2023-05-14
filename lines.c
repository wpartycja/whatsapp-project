#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "lines.h" 


// Funciones auxiliares

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
int recvMessage(int socket, char *buffer, int len) {
    int r;
    int l = len;
    
    do {
        r = read(socket, buffer, l); // read from socket
        
        // Check if the received data contains null character
        void *nullCharPtr = memchr(buffer, '\0', r);
        if (nullCharPtr != NULL) {
            r = ((char*)nullCharPtr - buffer) + 1;  // Update r to include the null character
            break;
        }
        
        l = l - r;
        buffer = buffer + r;
    } while ((l > 0) && (r >= 0));
    
    if (r < 0)
        return (-1);   // failure 
    else
        return (0);    // message received 
} */


int recvMessage(int socket, char *buffer, int len){
	int r;
	int l = len;
		
	do {	
		r = read(socket, buffer, l); // read from socket
		l = l -r ;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);   // fallo 
	else
		return(0);	// full length has been receive 
}


ssize_t readLine(int fd, void *buffer, size_t n){
	ssize_t numRead;  /* num of bytes fetched by last read() */
	size_t totRead;	  /* total bytes read so far */
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
		numRead = read(fd, &ch, 1);	/* read a byte */
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
            if (errno == EINTR)	/* interrupted -> restart read() */
        		continue;
        	else
				return -1;		/* some other error */
		} else if (numRead == 0) {	/* EOF */
			printf("numread 0\n");
        	if (totRead == 0)	/* no byres read; return 0 */
           		return 0;
			else
          		break;
        } else {			/* numRead must be 1 if we get here*/
        	if (ch == '\n')
           		break;
        	if (ch == '\0'){
				//printf("Es barra 0\n");
          		break;
			}
            if (totRead < n - 1) {		/* discard > (n-1) bytes */
				totRead++;
				*buf++ = ch; 
			}
		} 
	}
	
	*buf = '\0';
    
	return totRead;
}