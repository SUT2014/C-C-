/*
 * socket.c
 *
 *  Created on: 19 Feb 2014
 *      Author: KumaranD
 */

#include "socket.h"

int make_named_socket (const char *filename)
{
	struct sockaddr_un name;
    int sock;
    char tmpFilename[TEMP_STRING_SIZE];
     
    /* Create local TCP socket. */
    sock = socket (PF_LOCAL, SOCK_STREAM, 0);
    if (sock < 0)
    {
    	LOG ("local socket creation error");
        exit (0);
    }
    sprintf(tmpFilename,"/tmp/%s",filename);
    //delete the filename if existing
    remove(tmpFilename);
    /* Bind a name to the socket. */
        name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, tmpFilename, sizeof (name.sun_path));
     
    /* The size of the address is
       the offset of the start of the filename,
       plus its length (not including the terminating null byte).
    */
    if (bind (sock, (struct sockaddr *) &name, SUN_LEN(&name)) < 0)
    {
    	printf("localsocketerror:%s-%s",strerror(errno),name.sun_path);
    	LOG ("local socket bind error");
        exit (0);
    }
    //change the permissions on the temp file so that everyone has access
    if (chmod(name.sun_path,S_IWUSR | S_IWGRP | S_IWOTH) != 0)
    {
    	LOG ("local socket bind error");
    	exit (0);
    }

    return sock;
}

//send on socket
int socket_send(int sockfd,char *buff)
{
	if (buff == NULL)
	{
		LOG("Attempting to send empty buffer");
		return FAILURE;
	}
	if (send(sockfd,buff,PAYLOAD_MAX,0) != PAYLOAD_MAX)
	{
		perror("send");
		return FAILURE;
	}
	return SUCCESS;
}

//read on socket
//buff not allocated, malloc as necessary
int socket_receive(int sockfd,char *buff)
{
	if (buff == NULL)
	{
		LOG("Attempting to receive empty buffer");
		return FAILURE;
	}

	if (recv(sockfd,buff,PAYLOAD_MAX,0) <= 0)
	{
		perror("recv");
		return FAILURE;
	}
	return SUCCESS;
}
