#ifndef _SERVICIOS_H
#define _SERVICIOS_H 

int register_client(char *name, char *username, char *birthdate);
int unregister_client(char *username);
int connect_client(char *username, char *ip, char *port);
int disconnect_client(char *username);

#endif