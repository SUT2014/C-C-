/*
 * listener.h
 *
 *  Created on: 14 Feb 2014
 *      Author: KumaranD
 */

#ifndef LISTENER_H_
#define LISTENER_H_

#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<sys/un.h>
#include<pthread.h>
#include<errno.h>

#define TRUE 1
#define FALSE (! TRUE)

#define SUCCESS TRUE
#define FAILURE FALSE

#define PAYLOAD_MAX 256
#define TEMP_STRING_SIZE 30
#define TID_STRING_SIZE 15
#define EXECUTOR_MAX 5  //limit number of executors
#define LISTENER_PORT 8899
#define SA struct sockaddr

//config file
#define CONFIG_FILE "listener.config"

//debugging info
#define DBG 0
#define DBG_PRINT  if(DBG)fprintf
#define DBG_FLUSH  if(DBG)fflush


//background task from background.c
void* background_update(void *);
int socket_send(int,char *);
int socket_receive(int,char *);

//logging file handle
FILE * g_LogFile;

//global socket handle for all communications from clients
// opened up on LISTENER_PORT
int g_sockfd;

// Executor data structure
typedef struct exec {
	char name[20];
	int sock_fd; //socket to connect to executors
	int n_of_Commands;
	char * commandList;
}Exec;

//array of executors
Exec *g_Exec_List[EXECUTOR_MAX];

//thread tid
pthread_t g_BackgrounThread,g_ClientThread;

//mutex lock for Exec structure
pthread_mutex_t g_Exec_Lock;

#endif /* LISTENER_H_ */
