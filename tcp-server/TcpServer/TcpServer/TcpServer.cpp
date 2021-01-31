// TcpServer.cpp : Defines the entry point for the console application.
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

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    sockaddr_in addrLocal = { 0 };
    addrLocal.sin_family = AF_INET;
    addrLocal.sin_port = htons(6789);
    inet_pton(AF_INET, ADDR_ANY, &addrLocal.sin_addr.s_addr);

    bind(sListen, (sockaddr *)&addrLocal, sizeof(addrLocal));
    listen(sListen, SOMAXCONN);

    printf("start listen...\n");

    sockaddr_in addrRemote = { 0 };
    int addrRemoteLen = sizeof(addrRemote);
    SOCKET sAccept = accept(sListen, (sockaddr *)&addrRemote, &addrRemoteLen);
    if (INVALID_SOCKET == sAccept)
    {
        printf("accept failed\n");
        WSACleanup();
        return 0;
    }

    int wndSize = 100;
    setsockopt(sAccept, SOL_SOCKET, SO_SNDBUF, (const char *)&wndSize, sizeof(wndSize));
    setsockopt(sAccept, SOL_SOCKET, SO_RCVBUF, (const char *)&wndSize, sizeof(wndSize));

    int i = 0;
    do 
    {
        char szRecvBuffer[BufferSize + 2] = { 0 }, szSendBuffer[BufferSize + 2] = { 0 };
        int nRecvLen = recv(sAccept, szRecvBuffer, BufferSize, 0);
        if (nRecvLen == SOCKET_ERROR)
        {
            printf("revc error occurd, break\n");
            break;
        }
        else if (nRecvLen == 0)
        {
            printf("client is shutdown or closed\n");
            break;
        }
        else
        {
            printf("recv buffer size=%d\n", nRecvLen);

            sprintf_s(szSendBuffer, BufferSize, "%s send back", szRecvBuffer);
            int nSendLen = send(sAccept, szSendBuffer, strlen(szSendBuffer), 0);
            printf("send buffer size=%d, i=%d\n", nSendLen, i);
        }

        if (i == 5)
        {
            do 
            {
                sprintf_s(szSendBuffer, BufferSize, "%s send back, i=%d", szRecvBuffer, i);
                int nSendLen = recv(sAccept, szSendBuffer, BufferSize, 0);
                printf("send buffer size=%d\n", nSendLen);
            } while (i++ < 8);
        }
    } while (i++ < 10);

    closesocket(sAccept);
    closesocket(sListen);

    WSACleanup();

    return 0;
}

