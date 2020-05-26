/*******************************************************************************
*                 Copyright (c) 2014 Kumaran D                                 *
********************************************************************************
* Title            : socket.c                                                  *
* Project          : Set of functions for C sockets                            *
* Description      : Communication libraries for windows to talk to Listener   *
*                                                                              *
* Revision History :                                                           *
*------------------------------------------------------------------------------*
* Date                Version        Author                Remarks             *
*------------------------------------------------------------------------------*
* 13th March 2014       0.1            KD      		     Initial version       *
*																			   *
*******************************************************************************/
/*******************************************************************************
*                        Include Files                                         *
*******************************************************************************/
#include "socket.h"

SOCKET create_connect_socket(char *ip_address)
{
    WSADATA wsaData;
	SOCKET Socket = INVALID_SOCKET;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        //printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
    Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (Socket == INVALID_SOCKET) {
        WSACleanup();
        return 0;
    }
    SOCKADDR_IN SockAddr;
	SockAddr.sin_port=htons(PORT);
	SockAddr.sin_family=AF_INET;
	SockAddr.sin_addr.s_addr=inet_addr(ip_address);

	while(1)
	{
	    // Connect to server.
        iResult = connect(Socket,(SOCKADDR*)(&SockAddr),sizeof(SockAddr));
        if (iResult == SOCKET_ERROR)
        {
            //printf("Waiting for listener to startup: %d\n", iResult);
            Sleep(SOCKET_RETRY_MSECS);
            continue;
        }
		else
		{
			//printf("\nconnected to the server..\n");
			break;
		}
	}
    return Socket;
}

int socket_send(SOCKET s, char * buff, int len)
{
    return send(s,buff,len,0);
}

int socket_receive(SOCKET s, char * buff, int len)
{
    return recv(s,buff,len,0);
}

