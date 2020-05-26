/*
 * listener.c
 *
 *  Created on: 14 Feb 2014
 *      Author: KumaranD
 *
 *  Main file for the listener application.
 *  	1. Connects to executors locally.
 *  	2. Waits for connection from client.
 *  	3. Forwards commands to executors.
 *  	4. Returns results to Client.
 */

#include "listener.h"

//logging function
inline void LOG(char *logString)
{
	time_t now;
	time(&now);

	if( g_LogFile != NULL )
	{
		DBG_PRINT(g_LogFile,"%s:",logString);
		DBG_PRINT(g_LogFile,"%s",ctime(&now));
		DBG_FLUSH(g_LogFile);
	}
	else
	{
		printf("%s: %s",logString,ctime(&now));
	}
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

//cleanup function
void listener_cleanup()
{
	int i,j;
	if(DBG)
	{
		fclose(g_LogFile);
	}
	close(g_sockfd);
	//kill the background task thread
	pthread_cancel(g_BackgrounThread);
	pthread_cancel(g_ClientThread);
	//free the executor list
	for (i=0;i<EXECUTOR_MAX;i++)
	{
		if (g_Exec_List[i] != NULL)
		{
			for (j=0;j<g_Exec_List[i]->n_of_Commands;j++)
			{
				//free(g_Exec_List[i]->commandList[j]);
				free(g_Exec_List[i]->commandList);
			}
		}
		if (g_Exec_List[i]->sock_fd != -1)
		{
			close(g_Exec_List[i]->sock_fd);
		}
		g_Exec_List[i]=NULL;
	}
	pthread_mutex_destroy(&g_Exec_Lock);
}

int g_exec_lock()
{
	return(pthread_mutex_lock(&g_Exec_Lock));
}

int g_exec_unlock()
{
	return(pthread_mutex_unlock(&g_Exec_Lock));
}
//setup local socket and return handle
int get_local_socket(const char *filename)
{
	struct sockaddr_un name;
	int sock;
	char tmpFilename[TEMP_STRING_SIZE];

	/* Create TCP socket. */
	sock = socket (PF_LOCAL, SOCK_STREAM, 0);
	if (sock < 0)
	{
	  	LOG ("local socket creation error");
	    exit (0);
	}
	sprintf(tmpFilename,"/tmp/%s",filename);
    name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, tmpFilename, sizeof (name.sun_path));

	/* The size of the address is
	   the offset of the start of the filename,
	   plus its length (not including the terminating null byte).
	*/
	if (connect(sock, (struct sockaddr *) &name, SUN_LEN(&name)) < 0)
	{
		printf("localsocketerror:%s-%s",strerror(errno),name.sun_path);
	   	LOG ("local socket connect error");
	    return -1;
	}
	return sock;
}


/* Load the config file
 * read each line and load the executors' details
 */
int load_config()
{
	FILE *conf;
	int i=0;
	char buff[PAYLOAD_MAX];
	conf= fopen(CONFIG_FILE,"r");
	if (!conf)
	{
		LOG("Error opening config file..exiting");
		return FALSE;
	}
	//lock the exec mutex before updating it.
	g_exec_lock();
    while (fscanf(conf, "%s", buff) != EOF)
    {
    	if (strcmp(buff,"[EXECUTORS]") == 0)
    	{
    		continue;
    	}
    	g_Exec_List[i] = (Exec *)malloc(sizeof(Exec));
    	if(g_Exec_List[i] == NULL)
    	{
    		LOG("Memory allocation error: exiting..");
    		g_exec_unlock();
    	    return FALSE;
    	}
    	strcpy(g_Exec_List[i]->name,buff);
    	//setup a socket to connect to the named executor socket
    	//for inactive executors -1 will be returned.
    	g_Exec_List[i]->sock_fd = get_local_socket(g_Exec_List[i]->name);
    	g_Exec_List[i]->commandList = NULL;
    	i++;
    }
    g_exec_unlock();
	//all the executor names have been added, now start the background thread
	if (pthread_create(&g_BackgrounThread,NULL,&background_update,NULL) != 0)
	{
		LOG("Background thread not created, exiting..");
		return FALSE;
	}
	LOG("Background thread created sucessfully");
	fclose(conf);
	return TRUE;
}

//Listener initialisation function
int listener_init()
{
	int i;
	struct sockaddr_in servaddr;
	if (DBG)
	{
		g_LogFile = fopen("listener.log","w");
		if (!g_LogFile)
		{
			LOG("Listener: Logging disabled, cannot open log file");
		}
	}
	//init the executor list
	for (i=0;i<EXECUTOR_MAX;i++)
	{
		g_Exec_List[i]=NULL;
	}
	//load config file
	if (load_config() == FALSE)
	{
		return FALSE;
	}
	//create a TCP socket
	g_sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(g_sockfd == -1)
	{
		LOG("Listener socket creation failed...");
		return FALSE;
	}
	else
	{
		LOG("Listener Socket successfully created..");
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port=htons(LISTENER_PORT);
	bind(g_sockfd,(SA*)&servaddr,sizeof(servaddr));
	if (listen(g_sockfd, 10) == -1)
	{
		LOG ("Failed to listen on Listener port");
		return FALSE;
	}
	if (pthread_mutex_init(&g_Exec_Lock,NULL) != 0)
	{
		LOG ("Exec Lock mutex init failed");
		return FALSE;
	}
	return TRUE;
}

//based on executor dispatch to the right place
int dispatch_command(char *buff)
{
	char TEMP_BUFF[PAYLOAD_MAX];
	int i=0,first;
	int exec_socket;
	unsigned long tid = (unsigned long) pthread_self();
	first = strcspn(buff," ");
	//prepare command line to be dispatched
	//<thread id><Command string>
	//thread id is required as an unique id to route back response
	bzero(TEMP_BUFF,PAYLOAD_MAX);
	sprintf(TEMP_BUFF,"%015lu%s",tid,buff+first+1);
	g_exec_lock();
	for (i=0;i<EXECUTOR_MAX;i++)
	{
		if (g_Exec_List[i])
		{
			if (strncasecmp(buff,g_Exec_List[i]->name,4) ==0)
			{
				//write to socket
				exec_socket = g_Exec_List[i]->sock_fd;
				if (socket_send(exec_socket,TEMP_BUFF) != SUCCESS)
				{
					LOG(("executor socket closed..returning"));
					printf ("%s",strerror(errno));
					//error with the socket
					close(g_Exec_List[i]->sock_fd);
					g_Exec_List[i]->sock_fd = -1;
					g_exec_unlock();
					return FALSE;
				}
				g_exec_unlock();
				return exec_socket;
			}
		}
	}
	g_exec_unlock();
	return FALSE;
}

//main client handler, there might be multiple clients
void* handleClient(void *fd)
{
	char buff[PAYLOAD_MAX];
	int listenerfd = *((int *)fd);
	int response_socket = FALSE;
	for(;;)
	{
		bzero(buff,PAYLOAD_MAX);
		if (socket_receive(listenerfd,buff) != SUCCESS)
		{
			LOG("Client socket closed..closing client thread");
			return FALSE;
		}
		LOG(buff);

		if(strncasecmp(buff,"list",4) == 0)
		{
			//reuse buff
			bzero(buff,PAYLOAD_MAX);
			strcpy(buff,"Executors available:");
			int i,j;
			g_exec_lock();
			for (i=0;i<EXECUTOR_MAX;i++)
			{
				if (g_Exec_List[i])
				{
					strcat(buff,g_Exec_List[i]->name);
					strcat(buff,"\n");
					//check for commands
					for(j=0;j<g_Exec_List[i]->n_of_Commands;j++)
					{
						//add tab and newline to beautify the display
						strcat(buff,"\t");
						strcat(buff,g_Exec_List[i]->commandList);
						strcat(buff,"\n");
					}
				}
			}
			g_exec_unlock();
			LOG(buff);
			if (socket_send(listenerfd,buff+TID_STRING_SIZE) != SUCCESS)
			{
				LOG(("Cannot write to client socket..closing client thread"));
				return FALSE;
			}
		}
		else if (strncasecmp(buff,"exit",4) == 0)
		{
			LOG("Client issued exit..closing client thread");
			return TRUE;
		}
		else
		{
			response_socket = dispatch_command(buff);
			if (response_socket == FALSE)
			{
				bzero(buff,PAYLOAD_MAX);
				LOG("Error while sending command to executor.");
				//responde with dispatch failure
				sprintf(buff,"%s","Command Dispatch Failure");
				if (socket_send(listenerfd,buff) != SUCCESS)
				{
					LOG(("Cannot write to client socket..closing client thread"));
					return FALSE;
				}
				//continue, no need to exit,continue running other client commands
				continue;
			}
			//command successfully sent, wait for response
			bzero(buff,PAYLOAD_MAX);
			if (socket_receive(response_socket,buff) != SUCCESS)
			{
				LOG("Executor socket closed..no response available");
				return FALSE;
			}
			//discard the TID and forward response to listener
			if (socket_send(listenerfd,buff+TID_STRING_SIZE) != SUCCESS)
			{
				LOG(("Cannot write to client socket..closing client thread"));
				return FALSE;
			}
		}
	}
	return(TRUE);
}
//Listener processor function
int listener_process()
{
	int listenerfd;
    //get into a loop to listen to clients
	while (1)
	{

	    listenerfd=accept(g_sockfd,(SA*)NULL,NULL);
        if(listenerfd < 0)
        {
    	    LOG("Error accepting connection from client");
    	    return (FALSE);
        }
        LOG("Client connection accepted, starting a thread");
        if (pthread_create(&g_ClientThread,NULL,&handleClient,&listenerfd) != 0)
        {
            LOG("Client thread not created, exiting..");
    	    return FALSE;
        }
	}//end while
	return TRUE;
}
//main function
int main()
{
	//call the init function
	if (listener_init() == FALSE)
	{
		LOG("Error during listener init");
		listener_cleanup();
		return(FALSE);
	}
	if (listener_process()== FALSE)
	{
		LOG("Error during listener process");
		listener_cleanup();
		return(FALSE);
	}
	LOG("its all hunky dory, exiting with normal status");
	listener_cleanup();
	return TRUE;
}

