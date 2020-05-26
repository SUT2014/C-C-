/*
 * background.c
 *
 *  Created on: 20 Feb 2014
 *      Author: KumaranD
 */

#include "background.h"
#include "listener.h"

/* background update task
 * Start contacting each executor
 * execute list command to update the local command list
 * if any executor is not alive, set the sockfd to -1
 * Commands will not be targeted to executors with sockfd=-1
 * sleep for a designated time so that the lock is not hogged
 */
void* background_update(void *arg)
{
	int i;
	char buff[PAYLOAD_MAX];
	unsigned long tid = (unsigned long) pthread_self();
	while(1)
	{
		LOG("Send heartbeat list command to waiting executors\n");
		g_exec_lock();
		for(i=0;i<EXECUTOR_MAX;i++)
		{
			if (g_Exec_List[i] == NULL)
			{
				continue;
			}
			bzero(buff,PAYLOAD_MAX);
			sprintf(buff,"%015lu%s",tid,LIST_COMMAND);
			if (g_Exec_List[i]->sock_fd == -1)
			{
				/*socket to executor not alive
				 * try a new connection, if not alive
				 * give a try after timeout
				 */
				g_Exec_List[i]->sock_fd = get_local_socket(g_Exec_List[i]->name);
				//if executor is down, definitely continue
				if (g_Exec_List[i]->sock_fd == -1)
				{
					continue;
				}
			}
			//executor is alive, list
			if (socket_send(g_Exec_List[i]->sock_fd,buff) != SUCCESS)
			{
				//error with the socket
				close(g_Exec_List[i]->sock_fd);
				g_Exec_List[i]->sock_fd = -1;
				continue;
			}
			//wait for the executor to return the result to list
			bzero(buff,PAYLOAD_MAX);
			if (socket_receive(g_Exec_List[i]->sock_fd,buff) != SUCCESS)
			{
				//error with the socket
				close(g_Exec_List[i]->sock_fd);
				g_Exec_List[i]->sock_fd = -1;
				continue;
			}
			LOG("received from executor");
			LOG(buff);
			//free before loading the commands
			if (g_Exec_List[i]->commandList != NULL)
			{
				free(g_Exec_List[i]->commandList);
				g_Exec_List[i]->n_of_Commands = 0;
			}
			//discard the TID in the response
			g_Exec_List[i]->commandList = (char *)malloc(strlen(buff)-TID_STRING_SIZE);
			strcpy(g_Exec_List[i]->commandList,buff+TID_STRING_SIZE);
			g_Exec_List[i]->n_of_Commands ++;
		}
		g_exec_unlock();
		sleep(BACKGROUND_TIMEOUT);
	}
	return NULL;
}
