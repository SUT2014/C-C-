/*
 * executor.c
 *
 *  Created on: 18 Feb 2014
 *      Author: KumaranD
 */


#include "executor.h"

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
// setup flock IPC
int setup_flock(char *flock_name)
{
	g_LSock_Handle = make_named_socket(flock_name);
	return TRUE;
}
//cleanup function
void executor_cleanup()
{
	Comm *tempnow, *tempnext;
	//clear off the data structure
    if (g_execComms != NULL)
    {
    	tempnow = g_execComms->CommandList;
    	while (tempnow != NULL)
    	{
    		tempnext = tempnow->next;
    		free(tempnow->commandString);
    		free(tempnow);
    		tempnow = tempnext;
    	}
    }
    //kill the background task thread
    pthread_cancel(g_ExecThread);
	if(DBG)
	{
		fclose(g_LogFile);
	}
	if(g_LSock_Handle != NULL)
	{
		close(g_LSock_Handle);
	}
}

//malloc memory, if there is an error, exit.
char * allocate_memory(int size)
{
	char *temp;
	temp = malloc(size);
	if(temp == NULL)
	{
		LOG("Memory allocation error: exiting..");
		executor_cleanup();
		exit(0);
	}
	return temp;
}

/* Load the config file
 * read each line and load the command details
 */
int load_config()
{
	FILE *conf;
	char buff[PAYLOAD_MAX];
	Comm *tempList=NULL;
	conf= fopen(CONFIG_FILE,"r");
	if (!conf)
	{
		LOG("Error opening config file..exiting");
		return FALSE;
	}
	while (fscanf(conf, "%s", buff) != EOF)
    {
    	if (strncasecmp(buff,"[NAME]",6) == 0)
    	{
    		if (fscanf(conf, "%s", buff) != EOF)
			{
    			setup_flock(buff);
			}
		continue;
    	}
    	else if (strncasecmp(buff,"[COMMANDS]",10) == 0)
    	{
    		g_execComms = (CommL*)allocate_memory(sizeof(CommL));
    		g_execComms->n_of_Commands=0;
    		while (fgets(buff,PAYLOAD_MAX,conf) != NULL)
    		{
    			if (g_execComms->n_of_Commands == 0)
    			{
    				//create the seed command list.
    				g_execComms->CommandList = (Comm*)allocate_memory(sizeof(Comm));
    				tempList = g_execComms->CommandList;
    			}
    			else
    			{
    				tempList->next = (Comm*)allocate_memory(sizeof(Comm));
    				tempList = tempList->next;
    			}
    			tempList->commandString = (char *)allocate_memory(sizeof(buff));
    			strcpy(tempList->commandString,buff);
    			tempList->next = NULL;
    			g_execComms->n_of_Commands++;
    		}
    	}
    }
	fclose(conf);
	return TRUE;
}

//Executor initialisation function
int executor_init()
{
	g_execComms=NULL;
	//load config file
	if (load_config() == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}
//returns command list as a csv string
void get_commands_csv(char *commandList)
{
	Comm *tempnow, *tempnext;
   	tempnow = g_execComms->CommandList;
   	while (tempnow != NULL)
   	{
   		tempnext = tempnow->next;
   		strcpy(commandList,tempnow->commandString);
   		tempnow = tempnext;
   	}
}

//execution thread
int *exec_thread(char *buff)
{
	FILE *fp_runner;
	char temp_str[PAYLOAD_MAX];
	int i=TID_STRING_SIZE;

	bzero(temp_str,PAYLOAD_MAX);
	sprintf(temp_str,"%s 2>&1",buff+TID_STRING_SIZE);
	//execute command using popen
	fp_runner = popen(temp_str,"r");

	//reuse buff
	bzero(buff+TID_STRING_SIZE,PAYLOAD_MAX-TID_STRING_SIZE);
	//append result
	while (fgets(temp_str,PAYLOAD_MAX,fp_runner) != NULL)
	{
		strcat(buff,temp_str);
	}
	pclose(fp_runner);
	if (strlen(buff)==TID_STRING_SIZE)
	{
		sprintf(buff+TID_STRING_SIZE,"%s","Error Executing command!!");
	}
	if (socket_send(g_LSock_Client,buff) != SUCCESS)
	{
		LOG(("Cannot write to client socket..closing client thread"));
		return FALSE;
	}
	return TRUE;
}

//client handler function
int handle_client()
{
	char buff[PAYLOAD_MAX];
	while (1)
	{
		bzero(buff,PAYLOAD_MAX);
		//read the length of the message from the socket
		if (socket_receive(g_LSock_Client,buff) != SUCCESS )
		{
			LOG("Listener has closed socket");
			return FALSE;
		}
		LOG("Received from Listener");
		LOG(buff);
		if (strncasecmp (buff+TID_STRING_SIZE,LIST_COMMAND,strlen(LIST_COMMAND)) == 0)
		{
			//form the command response
			get_commands_csv(buff+TID_STRING_SIZE);
			if (socket_send(g_LSock_Client,buff) != SUCCESS)
			{
				//error with the socket
				return FALSE;
			}
		}
		else
		{
			//run the command using a new thread
			//if (pthread_create(&g_ExecThread,NULL,&exec_thread,buff[i]) != 0)
			//{
			//	LOG("Execution thread not created, discarding executed");
			//	return FALSE;
			//}
			(*exec_thread)(buff);
		}
	}
	return TRUE;
}
//Executor processor function
int executor_process()
{
	//process the input on the local socket.
	LOG("Listening on local socket...");
	if (listen(g_LSock_Handle,5) < 0)
	{
		LOG("Error while listening on executor socket");
		close(g_LSock_Handle);
		return FALSE;
	}
	for(;;)
	{
		//accept a connection
		g_LSock_Client = accept (g_LSock_Handle, (struct sockaddr*)NULL, NULL);
		if (g_LSock_Client < 0)
		{
			printf ("Socket error : %s",strerror(errno));
			return FALSE;
		}
		LOG("Accepted Connection from Listener");
		//handle listener
		handle_client();
	}
    return TRUE;
}
//main function
int main()
{
	//call the init function
	if (executor_init() == FALSE)
	{
		LOG("Error during executor init");
		executor_cleanup();
		return(FALSE);
	}
	if (executor_process()== FALSE)
	{
		LOG("Error during listener process");
		executor_cleanup();
		return(FALSE);
	}
	LOG("its all hunky dory, exiting with normal status");
	executor_cleanup();
	return TRUE;
}

