#ifndef _SERVICIOS_H
#define _SERVICIOS_H 

int register_client(char *name, char *username, char *birthdate);
int unregister_client(char *username);
int connect_client(char *username, char *ip, char *port);
int check_messages(char *username, char *ip, char *port);
int disconnect_client(char *username);
int is_connected(char *username);
int connected_users(int client_sd);
int send_message(int client_sd, char *username, char *receiver, char *mssg);
int send_aux(int client_sd, char *username, char *receiver, char *mssg, char *id, char *ip, char *port);

#endif