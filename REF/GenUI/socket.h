#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#define WIN32_LEAN_AND_MEAN
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


#define PORT 8899
#define SOCKET_RETRY_MSECS 5000

SOCKET create_connect_socket();

#endif // SOCKET_H_INCLUDED
