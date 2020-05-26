/*
 * socket.h
 *
 *  Created on: 19 Feb 2014
 *      Author: KumaranD
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#define TRUE 1
#define FALSE (! TRUE)

#define SUCCESS TRUE
#define FAILURE FALSE

#define TEMP_STRING_SIZE 30

#define PAYLOAD_MAX 256

int make_named_socket (const char *);
int socket_send(int,char *);
int socket_receive(int,char *);

#endif /* SOCKET_H_ */
