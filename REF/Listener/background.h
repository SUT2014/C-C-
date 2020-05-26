/*
 * background.h
 *
 *  Created on: 20 Feb 2014
 *      Author: KumaranD
 */

#ifndef BACKGROUND_H_
#define BACKGROUND_H_
#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include <sys/un.h>

#define LIST_COMMAND "list"
#define BACKGROUND_TIMEOUT 200

void* background_update(void *);
int get_local_socket(const char *filename);

#endif /* BACKGROUND_H_ */
