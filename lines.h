#include <unistd.h>

int sendMessage(int socket, char *buffer, int len);
int recvMessage(int socket, char *buffer, int len);
ssize_t readLine(int socket, char *buffer, size_t maxlen);