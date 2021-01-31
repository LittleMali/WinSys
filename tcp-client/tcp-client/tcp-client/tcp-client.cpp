// tcp-client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define BufferSize 2048

int main()
{
    WSADATA wsaData = { 0 };
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int wndSize = 100;
    setsockopt(sClient, SOL_SOCKET, SO_SNDBUF, (const char *)&wndSize, sizeof(wndSize));
    setsockopt(sClient, SOL_SOCKET, SO_RCVBUF, (const char *)&wndSize, sizeof(wndSize));

    sockaddr_in addrSrv = { 0 };
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(6789);
    inet_pton(AF_INET, "127.0.0.1", &addrSrv.sin_addr.s_addr);

    int nConnRet = connect(sClient, (const sockaddr *)&addrSrv, sizeof(addrSrv));
    printf("connect ret=%d\n", nConnRet);
    
    int i = 0;
    do 
    {
        char szRecvBuffer[BufferSize + 2] = { 0 }, szSendBuffer[BufferSize + 2] = { 0 };
        sprintf_s(szSendBuffer, BufferSize, "client send data, i=%d", i);

        int nSendLen = send(sClient, szSendBuffer, strlen(szSendBuffer), 0);
        printf("send data=%s, retLen=%d\n", szSendBuffer, nSendLen);

        int nRecvLen = recv(sClient, szRecvBuffer, BufferSize, 0);
        printf("recv data=%s, retLen=%d\n", szRecvBuffer, nRecvLen);
        if (nRecvLen == SOCKET_ERROR)
        {
            printf("srv is error\n");
            break;
        }
        else if (nRecvLen == 0)
        {
            printf("srv is shutdown or closed\n");
            break;
        }

        if (i == 5)
        {
            printf("client call shutdown for send.\n");
            shutdown(sClient, SD_RECEIVE);

            printf("still recv after shutdown.\n");
            do 
            {
                int nRecvLen = send(sClient, szRecvBuffer, strlen(szRecvBuffer), 0);
                printf("recv data=%s, retLen=%d\n", szRecvBuffer, nRecvLen);
                if (nRecvLen == SOCKET_ERROR)
                {
                    printf("srv is error\n");
                    break;
                }
                else if (nRecvLen == 0)
                {
                    printf("srv is shutdown or closed\n");
                    break;
                }
            } while (i++ < 8);
        }

        Sleep(1000);

    } while (i++ < 10);

    closesocket(sClient);

    WSACleanup();
    return 0;
}

