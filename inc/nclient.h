#ifndef _sussnet_h_
#define _sussnet_h_

#define ERR_ConnectionTimeout   -1
#define ERR_AlreadyConnected    -2
#define ERR_NotConnected        -3
#define ERR_SendError           -4
#define ERR_ReadError           -5
#define ERR_ConnectError        -6


extern "C" {

int __stdcall  SNI_Connect(char *IPAddress,
                                int ListenPort,
                                char *Name,
                                int Notify
                               );

int __stdcall  SNI_Disconnect(void);
int __stdcall  SNI_DoProberCommand(char *CmdStr, char *RspStr);
int __stdcall  SNI_SendCommand(int Id, int CmdNum, char *Data);
int __stdcall  SNI_GetResponse(int *Id, int *ErrorCode, char *Data);
int __stdcall  SNI_GetNotification(int *NtfNum, char *Data);
void __stdcall  SNI_ShowDebug(int ShowIt);
void __stdcall  SNI_EnableErrorMsg(int EnableIt);

}

#endif
