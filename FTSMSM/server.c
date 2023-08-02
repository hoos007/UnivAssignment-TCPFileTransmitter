#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "../socket_utility.h"

#define	APP_RCV_BUF_SIZE	65495
FILE* fp;
HANDLE		hMutex;

void ErrorHandling(char* message);

DWORD WINAPI EchoServiceThread(void* arg) {
	SOCKET		hSock = (SOCKET)arg;
	int			nReceived, nSent;
	char		app_rBuffer[APP_RCV_BUF_SIZE];
	int fsize;
	int totalSendSize = 0;
	int sendFileSize;
	int seek = 0;

	WaitForSingleObject(hMutex, INFINITE);
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	ReleaseMutex(hMutex);

	do {
		WaitForSingleObject(hMutex, INFINITE);
		fseek(fp, seek, SEEK_SET);
		sendFileSize = fread(app_rBuffer, sizeof(char), sizeof(app_rBuffer), fp);
		seek = seek + sizeof(app_rBuffer);
		if (send(hSock, app_rBuffer, sendFileSize, 0) == SOCKET_ERROR) {
			sockError(0);
			break;
		}
		else
		{
			totalSendSize += sendFileSize;
			printf("send File : [%d/%d]\n", totalSendSize, fsize);
		}
		ReleaseMutex(hMutex);
	} while (totalSendSize < fsize);

	closesocket(hSock);
	printf("%d 소켓으로 서비스하던 Thread를 종료합니다\n", hSock);

	return 0;
}

int main(int argc, char* argv[]) {
	int			max_client = atoi(argv[3]);
	WSADATA		wsaData;
	SOCKET		hListenSock;
	SOCKET		hAcceptedSock;
	SOCKET* hSock = (SOCKET*)malloc(sizeof(SOCKET) * max_client);
	HANDLE		hCreatedThread;
	HANDLE* hThread = (HANDLE*)malloc(sizeof(HANDLE) * max_client);
	SOCKADDR_IN servAddr, clntAddr;
	int			nAddrLen = sizeof(clntAddr);
	DWORD		dwThreadID;
	int			nClient = 0;

	/* 0 */
	if (argc != 4) {
		printf("Usage : %s <port> <file_name> <max_client>\n", argv[0]);
		exit(1);
	}

	if ((hMutex = CreateMutex(NULL, FALSE, NULL)) == NULL)	ErrorHandling("뮤텍스 생성 오류");

	fp = fopen(argv[2], "rb");
	if (fp == NULL)
	{
		printf("파일 열기 실패 종료합니다.");
		exit(1);
	}

	/* 1 */
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)	ErrorHandling("WSAStartup() error!");

	/* 2 */
	if ((hListenSock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)	ErrorHandling("socket() error");

	memset(&servAddr, 0, sizeof(servAddr));
	sockSetAddress(&servAddr, NULL, atoi(argv[1]));
	if (bind(hListenSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)	ErrorHandling("bind() error");

	if (listen(hListenSock, 5) == SOCKET_ERROR)	ErrorHandling("listen() error");

	printf("service started on port %d, Up to %d clients will be serviced\n", atoi(argv[1]), max_client);

	/* 3 */
	while (nClient < max_client) {
		if ((hAcceptedSock = accept(hListenSock, (SOCKADDR*)&clntAddr, &nAddrLen)) == INVALID_SOCKET) {
			puts("accept() error");
			break;
		}

		if ((hCreatedThread = CreateThread(NULL, 0, EchoServiceThread, (void*)hAcceptedSock, 0, &dwThreadID)) == 0) {
			puts("CreateThread error");
			break;
		}

		printf("%dth Client [%s:%d]에 대해 %d 소켓으로 Echo 서비스를 시작하였습니다\n",
			nClient + 1, inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port), hAcceptedSock);

		hSock[nClient] = hAcceptedSock;
		hThread[nClient] = hCreatedThread;
		nClient++;
	}


	UINT nWait = WaitForMultipleObjects(nClient, hThread, TRUE, INFINITE);

	CloseHandle(hMutex);


	/* 4 */
	closesocket(hListenSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);

	exit(1);
}