/*******************************************************************************
*                 Copyright (c) 2014 Kumaran Devaneson                         *
********************************************************************************
* Title            : main.c                                                    *
* Project          : GenUI (MOAT)                                              *
* Description      : Generic UI rendered from an xml file.                     *
*                                                                              *
* Revision History :                                                           *
*------------------------------------------------------------------------------*
* Date                Version        Author                Remarks             *
*------------------------------------------------------------------------------*
* Mar 31 2014           0.1            KD      		     Initial version       *
*																			   *
*******************************************************************************/
/*******************************************************************************
*                        Include Files                                         *
*******************************************************************************/
#undef UNICODE
#include <windows.h>
#include <commctrl.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <windows.h>
#include <commctrl.h>
#include "resource.h"

/*******************************************************************************
*                        Globals                                               *
*******************************************************************************/
char g_cmd_line[PAYLOAD_MAX];
char g_title[PAYLOAD_MAX] = "Generic UI";
char g_logfile_name[PAYLOAD_MAX] = "GenericUI.log";
char g_xml_filename[PAYLOAD_MAX];
char g_ip_address[PAYLOAD_MAX];
SOCKET g_listener_socket = 0;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HWND Window;
HWND StatusWindow,hStaticStatus;
HWND hButton;
HWND hWndComboBox;
ParameterWindow paramEdit[MAX_PARAM_COUNT];
HINSTANCE hiInst;
boolean running_script=FALSE;
HANDLE hThread;
DWORD threadID;
FILE *logFile = NULL;

void logger(const char *logString)
{
    time_t now;
    time(&now);
    if (logFile != NULL)
    {
        fseek(logFile, 0L, SEEK_END);
        fprintf(logFile,"%s-----:%s\n",ctime(&now),logString);
        fflush(logFile);
    }
}
// create tab boxes.
/* Tool control */
void CreateToolBox()
{
    RECT tr;
    /* get the tab size info so
       we can place the view window
       in the right place */
    GetClientRect( Window, &tr );

    StatusWindow = CreateWindow(
                    "EDIT",
                    NULL,
                    WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL |
                    ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
                    0,
                    (tr.bottom/2+25),
                    tr.right,
                    (tr.bottom/3+25),
                    Window,
                    NULL,
                    hiInst,
                    NULL
                    );

    if (StatusWindow == NULL)
    {
          MessageBox(NULL, "Status window creation failed", "", MB_OK );
          return;
    }
    if (ScrollWindowEx(StatusWindow,DEFAULT_SCROLL_HORIZONTAL,1024,NULL,NULL,NULL,NULL,0) == NULL)
    {
          MessageBox(NULL, "Scroll window creation failed", "", MB_OK );
          return;
    }
}

int parse_xml(char *filename)
{
    FILE *fp;
    char lineIn[PAYLOAD_MAX];
    char temp_str[PAYLOAD_MAX];
    int i = 0,j=0,m,n;
    if ((fp = fopen(filename,"r")) != NULL)
    {
        while (fgets(lineIn,PAYLOAD_MAX,fp) != NULL)
        {
            //search for catalog
            if (strstr(lineIn,"<CATALOG>") == NULL) continue;
            //catalog found
            do
            {
                if (strstr(lineIn,"<TITLE>") != NULL)
                {
                    m = strcspn(lineIn,">");
                    n = strcspn(lineIn+m,"<");
                    memset(g_title, 0, PAYLOAD_MAX);
                    strncpy(g_title,lineIn+m+1,n-1);
                    continue;
                }
                //retrieve ip address of listener to connect
                if (strstr(lineIn,"<IP>") != NULL)
                {
                    m = strcspn(lineIn,">");
                    n = strcspn(lineIn+m,"<");
                    memset(g_ip_address, 0, PAYLOAD_MAX);
                    strncpy(g_ip_address,lineIn+m+1,n-1);
                    continue;
                }
                //search for <command>
                if (strstr(lineIn,"<COMMAND>") == NULL) continue;
                //command found
                do
                {
                    //search for <name>
                    if (strstr(lineIn,"<NAME>") != NULL)
                    {
                        m = strcspn(lineIn,">");
                        n = strcspn(lineIn+m,"<");
                        strncpy(CommandArray[i].commandName,lineIn+m+1,n-1);
                    }
                    else if (strstr(lineIn,"<DESCRIPTION>") != NULL)
                    {
                        m = strcspn(lineIn,">");
                        n = strcspn(lineIn+m,"<");
                        strncpy(CommandArray[i].commandDesc,lineIn+m+1,n-1);
                    }
                    else if (strstr(lineIn,"<COMMAND_WORD>") != NULL)
                    {
                        m = strcspn(lineIn,">");
                        n = strcspn(lineIn+m,"<");
                        strncpy(CommandArray[i].commandWord,lineIn+m+1,n-1);
                    }
                    else if (strstr(lineIn,"<CONFIRM>") != NULL)
                    {
                        m = strcspn(lineIn,">");
                        n = strcspn(lineIn+m,"<");
                        strncpy(CommandArray[i].commandConfirm,lineIn+m+1,n-1);
                    }
                    else if (strstr(lineIn,"</COMMAND>") != NULL) break;
                    else if (strstr(lineIn,"<PARAMETER>") != NULL)
                    {
                        memset(temp_str, 0, PAYLOAD_MAX);
                        do
                        {
                            //search for parameters
                            if (strstr(lineIn,"<POSITION>") != NULL)
                            {
                                m = strcspn(lineIn,">");
                                n = strcspn(lineIn+m,"<");
                                strncpy(temp_str,lineIn+m+1,n-1);
                                j = atoi(temp_str);
                                memset(temp_str, 0, PAYLOAD_MAX);
                            }
                            if (strstr(lineIn,"<NAME>") != NULL)
                            {
                                m = strcspn(lineIn,">");
                                n = strcspn(lineIn+m,"<");
                                strncpy(temp_str,lineIn+m+1,n-1);
                            }
                            else if (strstr(lineIn,"</PARAMETER>") != NULL)
                            {
                                strcpy(CommandArray[i].ParameterArray[j-1].parameterName,temp_str);
                                break;
                            }
                        }while (fgets(lineIn,PAYLOAD_MAX,fp) != NULL);
                    }
                }while (fgets(lineIn,PAYLOAD_MAX,fp) != NULL);
                i++;
                if (strstr(lineIn,"</CATALOG>") != NULL) break;
            }while (fgets(lineIn,PAYLOAD_MAX,fp) != NULL);
        }
    }
    return i;
    fclose(fp);
}
//destroy parameter details.
void destroy_param_region()
{
    int i;
    for (i=0;i<MAX_PARAM_COUNT;i++)
    {
        if (paramEdit[i].paramName != NULL)
        {
            DestroyWindow(paramEdit[i].paramEdit);
            DestroyWindow(paramEdit[i].paramName);
            paramEdit[i].paramEdit = paramEdit[i].paramName = NULL;
        }

    }
}
//render the parameters from the list.
void display_parameters(int commandIndex)
{
    RECT tr;
    int i;
    UpdateWindow(Window);
    GetClientRect( hWndComboBox, &tr );
    destroy_param_region();
    for (i=0;i<MAX_PARAM_COUNT;i++)
    {
        int len = strlen(CommandArray[commandIndex].ParameterArray[i].parameterName);
        if (len > 0)
        {
            paramEdit[i].paramName = CreateWindowEx(0, "STATIC", CommandArray[commandIndex].ParameterArray[i].parameterName, WS_CHILD | WS_VISIBLE,
                            tr.left+10, tr.bottom+85+(i*30), 130, 20, Window, (HMENU)IDC_STATIC_1, GetModuleHandle(NULL), NULL);
            if(paramEdit[i].paramName == NULL)
            {
                MessageBox(Window, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);
            }
            paramEdit[i].paramEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                tr.left+180, tr.bottom+85+(i*30), 130, 20, Window, (HMENU)IDC_EDIT_CHANNEL, GetModuleHandle(NULL), NULL);
            if(paramEdit[i].paramEdit == NULL)
            {
                MessageBox(Window, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);
            }
            SetWindowText(paramEdit[i].paramEdit, paramEdit[i].paramValue);
            EnableWindow( paramEdit[i].paramName, TRUE );
            EnableWindow( paramEdit[i].paramEdit, TRUE );
            SendMessage(paramEdit[i].paramEdit, EM_SETLIMITTEXT, PAYLOAD_MAX, 0);
        }
    }//endfor
}
//render the parameters from the list.
void set_parameters(int commandIndex,boolean state)
{
    int i;
    UpdateWindow(Window);
    for (i=0;i<MAX_PARAM_COUNT;i++)
    {
        int len = strlen(CommandArray[commandIndex].ParameterArray[i].parameterName);
        if (len > 0)
        {
            EnableWindow(paramEdit[i].paramEdit,state);
        }
    }//endfor
}
//find a character and return index
int find_char(char * str, char character)
{
    int i;
    for (i=0;i<strlen(str);i++)
    {
        if (str[i] == character)
        {
            return i;
        }
    }
}
//insert string
void insert_string(char * string1,char * string2,int pos,int displacement)
{
    char temp[PAYLOAD_MAX];

    memset(temp,0,PAYLOAD_MAX);
    strcpy(temp,string1);
    strncpy(string1+pos,string2,strlen(string2));
    strncpy(string1+pos+strlen(string2),temp+pos+1+displacement,strlen(temp)-pos);
}
//add parameters to the command line
void substitute_parameters(char * commandLine)
{
    char temp2[PAYLOAD_MAX];
    int i;
    while ((i=find_char(commandLine,'$')) < strlen(commandLine))
    {
        int index = *(commandLine+i+1) - '0';
        GetWindowText(paramEdit[index-1].paramEdit, (LPTSTR)temp2, PAYLOAD_MAX);
        insert_string(commandLine,temp2,i,1);
    }
}
//get the first xml filename in the current directory.
char * getXMLFileName()
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;
    hFind = FindFirstFile("t*.xml", &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        MessageBox(hWndComboBox, "Config XML file not found, exiting !!", "Error", MB_ICONSTOP);
        DestroyWindow(Window);
        ExitProcess(0);
    }
    else
    {
        strcpy(g_xml_filename,FindFileData.cFileName);
        FindClose(hFind);
        return(g_xml_filename);
    }

}
//beautify the tab boxes with required designs.
void BeautifyTabBox()
{
    RECT tr;

    GetClientRect( Window, &tr );
    int i,nOfCommands = parse_xml(getXMLFileName());
    SetWindowText(Window, g_title);
    CreateWindowEx(0, "STATIC", "Command List:",
		WS_CHILD | WS_VISIBLE,
		tr.left+15, tr.top+45, 400, 100, Window, (HMENU)IDC_STATIC_1, GetModuleHandle(NULL), NULL);
    hWndComboBox = CreateWindow("COMBOBOX",
                                        NULL,
                                        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_DISABLENOSCROLL ,
                                        tr.left+120, tr.top+40, 500, 100,
                                        Window,
                                        NULL,
                                        hiInst,
                                        NULL);
    for (i=0;i<nOfCommands;i++)
    {
        SendMessage(hWndComboBox,
                        CB_ADDSTRING,
                        0,
                        &CommandArray[i]);
    }
    //pick the first item as default
    SendMessage(hWndComboBox, CB_SETCURSEL, 0, 0);
    SetWindowText(StatusWindow, CommandArray[0].commandDesc);
    display_parameters(0);
    hButton = CreateWindowEx(0, "BUTTON", "Send", WS_CHILD | WS_VISIBLE, 400, 200, DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT, Window,
        (HMENU)IDC_BUTTON, GetModuleHandle(NULL), NULL);
}

// Dialog procedure for our "about" dialog.
INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
        {
          EndDialog(hwndDlg, (INT_PTR) LOWORD(wParam));
          return (INT_PTR) TRUE;
        }
      }
      break;
    }

    case WM_INITDIALOG:
      return (INT_PTR) TRUE;
  }

  return (INT_PTR) FALSE;
}

/*void appendLogText(LPCTSTR newText)
{
  DWORD l,r;
  SendMessage(StatusWindow, EM_SETSEL, -1, -1);
  SendMessage(StatusWindow, EM_REPLACESEL, 0, (LPARAM)newText);
}*/
void appendLogText(LPCTSTR newText)
{
   int ndx;
   ndx = GetWindowTextLength (StatusWindow);
   //ndx+= strlen(newText);
   SetFocus (StatusWindow);
   SendMessage (StatusWindow, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
   SendMessage (StatusWindow, EM_REPLACESEL, 0, (LPARAM) ((LPSTR) newText));
   UpdateWindow(StatusWindow);
}
//dispatch a command and retrieve result.
int32_t send_and_receive(const char * commandline, boolean confirm)
{
    char mess[PAYLOAD_MAX];
    if (strlen(commandline) < 4 ) return 1; //ignore if blanks or invalid commands
    sprintf(mess, "%s - %s", commandline,"Retrieving result..please wait");
    SetWindowText(hStaticStatus,mess);
    //dispatch command
	if(socket_send(g_listener_socket, commandline,PAYLOAD_MAX) ==  SOCKET_ERROR)
    {
        MessageBox(hWndComboBox, "Command dispatch failure!!", "Error", MB_ICONSTOP);
        return -1;
    }
    else
    {
        if (confirm == TRUE)
        {
            MessageBox(hWndComboBox, "Command dispatched successfully", "Success", MB_OK);
        }
        logger(commandline);
    }
    //receive the result, reuse commandline
    memset(commandline, 0, PAYLOAD_MAX);
    if(socket_receive(g_listener_socket, commandline,PAYLOAD_MAX) ==  SOCKET_ERROR)
    {
        SetWindowText(StatusWindow, "Error receiving result from server");
    }
    else
    {
        appendLogText(commandline);
        appendLogText("\n#############################################\n");
    }
	return 1;
}
//dispatch a command and retrieve result.
int32_t send_and_receive2(SOCKET sockHandle,const char * commandline, boolean confirm)
{
    char mess[PAYLOAD_MAX];
    if (strlen(commandline) < 4 ) return 1; //ignore if blanks or invalid commands
    sprintf(mess, "%s - %s", commandline,"Retrieving result..please wait");
    SetWindowText(hStaticStatus,mess);
    //dispatch command
	if(socket_send(sockHandle, commandline,PAYLOAD_MAX) ==  SOCKET_ERROR)
    {
        MessageBox(hWndComboBox, "Command dispatch failure!!", "Error", MB_ICONSTOP);
        return -1;
    }
    else
    {
        if (confirm == TRUE)
        {
            MessageBox(hWndComboBox, "Command dispatched successfully", "Success", MB_OK);
        }
        logger(commandline);


        UpdateWindow(StatusWindow);


    }
    //receive the result, reuse commandline
    memset(mess, 0, PAYLOAD_MAX);
    if(socket_receive(sockHandle, mess,PAYLOAD_MAX) ==  SOCKET_ERROR)
    {
        SetWindowText(StatusWindow, "Error receiving result from server");
    }
    else
    {
        appendLogText(mess);
        appendLogText("\n#############################################\n");
    }
	return 1;
}
int run_remote_command(char *commandLine)
{
    SOCKET remote_socket;
    char temp[30];
    int ret;
    strncpy(temp,commandLine,30);
    remote_socket = create_connect_socket(strtok(temp," "));
    ret = send_and_receive2(remote_socket,strchr(commandLine,' ')+1,FALSE);
    closesocket(remote_socket);
    return ret;
}
//user pressed the button, go nuts !!
void __stdcall handle_send_button(void *p)
{
    int i,delay;
    char temp_str[PAYLOAD_MAX];
    char * filename;
    SetWindowText(StatusWindow, "");
	SetWindowText(hStaticStatus, "Sending...please wait");
	FILE *scriptFile;
	int commandIndex = (int)p;
    EnableWindow( hButton, FALSE );
    EnableWindow(hWndComboBox, FALSE);
    set_parameters(commandIndex,FALSE);
    //prepare the command string to dispatch
    memset(g_cmd_line, 0, PAYLOAD_MAX);
    memset(temp_str, 0, PAYLOAD_MAX);
    sprintf(g_cmd_line, "%s", CommandArray[commandIndex].commandWord);
    //block successive calls till current run ends.
    running_script = TRUE;
    //if it is not a script, pass the single command with params.
    if (strstr (g_cmd_line,"SCRIPT") == NULL)
    {
        //add parameters
        for(i=0;i<MAX_PARAM_COUNT;i++)
        {
            memset(temp_str, 0, PAYLOAD_MAX);
            if (strlen(CommandArray[commandIndex].ParameterArray[i].parameterName)>0)
            {
                GetWindowText(paramEdit[i].paramEdit, (LPTSTR)temp_str, PAYLOAD_MAX);
                strcpy(paramEdit[i].paramValue,temp_str);
                sprintf(g_cmd_line, "%s %s",g_cmd_line,temp_str);
            }
        }
        appendLogText(g_cmd_line);
        appendLogText("\n");
        SetWindowText(hStaticStatus,g_cmd_line);
        //if ip address provided, run remote execute
        if (isdigit(g_cmd_line[0]))
        {
            run_remote_command(g_cmd_line);
        }
        else //run locally
        {
            send_and_receive(g_cmd_line,TRUE);
        }
    }
    else //script, run through with it.
    {
        filename = ((strchr(g_cmd_line,' '))+1);
        scriptFile = fopen(filename,"r");
        if (scriptFile != NULL)
        {
            while( fgets(g_cmd_line, PAYLOAD_MAX, scriptFile ) != NULL)
            {
                UpdateWindow(StatusWindow);
                substitute_parameters(g_cmd_line);
                appendLogText(g_cmd_line);
                appendLogText("\n");
                SetWindowText(hStaticStatus,g_cmd_line);
                //if sleep, then run it locally
                if (strstr (g_cmd_line,"SLEEP") != NULL)
                {
                    delay = atoi((strchr(g_cmd_line,' '))+1);
                    Sleep(delay*1000); //seconds - > milli seconds
                }
                else
                {
                    //if ip address provided, run remote execute
                    if (isdigit(g_cmd_line[0]))
                    {
                        if (run_remote_command(g_cmd_line) == -1)
                        {
                            MessageBox(hWndComboBox, "Command run failure!!", "Error", MB_ICONSTOP);
                            fclose(scriptFile);
                            running_script = FALSE;
                            EnableWindow(hButton,TRUE );
                            EnableWindow(hWndComboBox, TRUE);
                            set_parameters(commandIndex,TRUE);
                            SetWindowText(hStaticStatus, "Pick a Command and press Send");
                            return;
                        }
                    }
                    else //run locally
                    {
                        if (send_and_receive(g_cmd_line,FALSE) == -1)
                        {
                            MessageBox(hWndComboBox, "Command run failure!!", "Error", MB_ICONSTOP);
                            fclose(scriptFile);
                            running_script = FALSE;
                            EnableWindow(hButton,TRUE );
                            EnableWindow(hWndComboBox, TRUE);
                            set_parameters(commandIndex,TRUE);
                            SetWindowText(hStaticStatus, "Pick a Command and press Send");
                            return;
                        }
                    }
                }
                memset(g_cmd_line, 0, PAYLOAD_MAX);
            }
            MessageBox(hWndComboBox, "Command run successfully", "Success", MB_OK);
        }
        else
        {
            memset(temp_str, 0, PAYLOAD_MAX);
            sprintf(temp_str,"%s..%s not found!!","Command run failure",filename);
            MessageBox(hWndComboBox, temp_str, "Error", MB_ICONSTOP);
        }
    }
    fclose(scriptFile);
    running_script = FALSE;
    EnableWindow(hButton,TRUE );
    EnableWindow(hWndComboBox, TRUE);
    set_parameters(commandIndex,TRUE);
    SetWindowText(hStaticStatus, "Pick a Command and press Send");
}

void start_thread(commandIndex)
{
    hThread = CreateThread(NULL, // security attributes ( default if NULL )
                                                0, // stack SIZE default if 0
                                                handle_send_button, // Start Address
                                                (void*)commandIndex, // input data
                                                0, // creational flag ( start if  0 )
                                                &threadID); // thread ID

}
BOOLEAN confirm_set(int ItemIndex)
{
    if (strlen(CommandArray[ItemIndex].commandConfirm) != 0)
    {
        if (MessageBox(hWndComboBox, CommandArray[ItemIndex].commandConfirm, "Wait", MB_YESNO | MB_ICONWARNING) == IDNO)
        {
            return FALSE;
        }
    }
    return TRUE;
}
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

   switch(msg) {

       case WM_DESTROY:
            PostQuitMessage(0);
            break;
       case WM_SETFOCUS:
            SetFocus(StatusWindow);
            return 0;
            break;
       case WM_COMMAND: // Windows Controls processing
       {
            switch(HIWORD(wParam))
            {
                case CBN_SELCHANGE: // This means a selection has been made
                {
                    //get the index of the item selected
                    int ItemIndex = SendMessage(hWndComboBox, (UINT) CB_GETCURSEL,
                                                        (WPARAM) 0, (LPARAM) 0);
                    //highlight respective command description
                    SetWindowText(StatusWindow, CommandArray[ItemIndex].commandDesc);
                    display_parameters(ItemIndex);
                }
                break;
            }
            switch (LOWORD(wParam))
		    {
			    case ID_HELP_ABOUT:
			    {
				    DialogBox(hiInst, MAKEINTRESOURCE(IDD_ABOUTDIALOG), Window, &AboutDialogProc);
				    return 0;
			    }

			    case ID_FILE_EXIT:
			    {
				    DestroyWindow(Window);
				    return 0;
			    }

			    case IDC_BUTTON:
			    {
			        //get the index of the item selected
                    int ItemIndex = SendMessage(hWndComboBox, (UINT) CB_GETCURSEL,
                                                        (WPARAM) 0, (LPARAM) 0);
                    if (running_script == FALSE)
                    {
                        if (confirm_set(ItemIndex) == FALSE)
                        {
                            return 0;
                        }
                        start_thread(ItemIndex);
                    }

				    return 0;
			    }
		    }
		    break;
		}
	   case WM_NOTIFY:
        {
            return 0;
        }
 		break;
   }
   return(DefWindowProc(hwnd, msg, wParam, lParam));
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow )
{
    MSG  msg ;
    WNDCLASSEX wc = {0};
    LPCTSTR MainWndClass = TEXT("GenUI V1.0");
    HACCEL hAccelerators;
    HMENU hSysMenu;
    char mess[PAYLOAD_MAX];

    char *pch;

	strncpy((char *)g_cmd_line, (char *)lpCmdLine, 1024);
	pch = strtok ((char*)g_cmd_line," ");
	if(pch != NULL)
	{
		strncpy((char*)g_ip_address, (char*)pch, 512);
	}
	else
    {
        strcpy((char*)g_ip_address, SERVER_IP);
    }


    // Class for our main window.
	wc.cbSize        = sizeof(wc);
	wc.style         = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc   = &MainWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_SHARED);
	wc.hCursor       = (HCURSOR) LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = MainWndClass;
	wc.hIconSm       = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_SHARED);
    // Register our window classes, or error.
	if (! RegisterClassEx(&wc))
	{
		MessageBox(NULL, TEXT("Error registering window class."), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 0;
	}
    RegisterClass(&wc);
    Window = CreateWindow(wc.lpszClassName,
                          TEXT("GenUI v0.1"),
						  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
						  300,
						  300,
						  DEFAULT_WINDOW_WIDTH,
						  DEFAULT_WINDOW_HEIGHT,
						  NULL,
						  NULL,
						  hInstance,
						  NULL);
    if (Window == NULL)
    {
        MessageBox(NULL, "Main window creation failed", "", MB_OK );
        return -1;
    }

    hStaticStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", Window, 9000);

    // Load accelerators.
	hAccelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	// Add "about" to the system menu.
	hSysMenu = GetSystemMenu(Window, FALSE);
	InsertMenu(hSysMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenu(hSysMenu, 6, MF_BYPOSITION, ID_HELP_ABOUT, TEXT("About"));

    CreateToolBox();
    BeautifyTabBox();
    // Show window and force a paint.
    ShowWindow(Window, nCmdShow);
    UpdateWindow(Window);
    //try connecting to listener
    EnableWindow( hButton, FALSE );
    sprintf(mess,"%s %s","Connecting to listener on:",g_ip_address);
    SetWindowText(hStaticStatus, mess);
    g_listener_socket = create_connect_socket(g_ip_address);
    sprintf(mess,"%s - %s %s",g_title,"Connected to:",g_ip_address);
    SetWindowText(Window, mess);
    SetWindowText(hStaticStatus, "Pick a Command and press Send");
    EnableWindow( hButton, TRUE );
    EnableWindow(hWndComboBox, TRUE);
    // open the log file for appending
    sprintf(g_logfile_name,"%s%s",g_title,".log");
    if ((logFile = fopen(g_logfile_name,"a+")) == NULL)
    {
        MessageBox(Window, "Log file creation error, logging disabled", "Error", MB_ICONSTOP );
    }
    //log connection details
    logger(mess);
    // Main message loop.
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		if (! TranslateAccelerator(msg.hwnd, hAccelerators, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
 }

