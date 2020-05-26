#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif
#define IDI_APPICON                     101
#define IDI_APPICON2                     105
#define IDR_MAINMENU                    102
#define IDD_ABOUTDIALOG                 103
#define IDR_ACCELERATOR                 104
#define ID_HELP_ABOUT                   40001
#define ID_FILE_EXIT                    40002

#define IDC_EDIT_CHANNEL				201
#define IDC_EDIT_CHAN_ID				202

#define IDC_STATIC_1					301
#define IDC_STATIC_2					302
#define IDC_STATIC_3					303
#define IDC_STATIC_4					304
#define IDC_STATIC_5					305
#define IDC_STATIC_6					306
#define IDC_BUTTON						401
#define STRING_LIMIT 30
#define DLG_MAIN                                100

#define ID_TABCTRL 1
#define EDIT 2
#define BTN_ADD 3
#define BTN_DEL 4
#define BTN_DELALL 5

/* Global constant */
#define DEFAULT_BUTTON_WIDTH 150
#define DEFAULT_BUTTON_HEIGHT 30
#define DEFAULT_WINDOW_WIDTH 650
#define DEFAULT_WINDOW_HEIGHT 500
#define DEFAULT_SCROLL_HORIZONTAL 100
#define DEFAULT_SCROLL_VERTICAL 200

#define MAX_PARAM_COUNT 5
#define MAX_COMMAND_COUNT 10

#define PAYLOAD_MAX 1024

//#define SERVER_IP "10.65.142.187"
#define SERVER_IP "172.22.27.131"

typedef struct param
{
    char parameterName[PAYLOAD_MAX];
    HWND paramEdit;
    HWND paramName;
    HWND paramValue;
}Parameter;

typedef struct comm
{
    char commandName[PAYLOAD_MAX];
    char commandDesc[PAYLOAD_MAX];
    char commandWord[PAYLOAD_MAX];
    char commandConfirm[PAYLOAD_MAX];
    Parameter ParameterArray[MAX_PARAM_COUNT];
}Command;

Command CommandArray[MAX_COMMAND_COUNT];

typedef struct paramW
{
    HWND paramEdit;
    HWND paramName;
    char paramValue[PAYLOAD_MAX];
}ParameterWindow;



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
