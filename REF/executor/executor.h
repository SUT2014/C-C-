/*
 * executor.h
 *
 *  Created on: 18 Feb 2014
 *      Author: KumaranD
 */

#ifndef EXECUTOR_H_
#define EXECUTOR_H_
#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include <stddef.h>
#include <errno.h>
#include <sys/un.h>
#include<pthread.h>

#define TRUE 1
#define FALSE (! TRUE)

#define SUCCESS TRUE
#define FAILURE FALSE

#define PAYLOAD_MAX 256
#define TEMP_STRING_SIZE 30
#define TID_STRING_SIZE 15
#define EXEC_THREAD_MAX 5

//config file
#define CONFIG_FILE "executor.config"

//debugging info
#define DBG 0
#define DBG_PRINT  if(DBG)fprintf
#define DBG_FLUSH  if(DBG)fflush

#define LIST_COMMAND "list"

//logging file handle
FILE * g_LogFile=NULL;

typedef struct comm {
	char * commandString;
	struct comm * next;
}Comm;

typedef struct commL {
	int n_of_Commands;
	Comm *CommandList;
}CommL;

CommL *g_execComms=NULL;

int g_LSock_Handle=0;
int g_LSock_Client=0;

int *exec_thread(char *);
//from socket.c
int make_named_socket (const char *);
int socket_send(int,char *);
int socket_receive(int,char *);

//thread tid
pthread_t g_ExecThread;


#endif /* EXECUTOR_H_ */
