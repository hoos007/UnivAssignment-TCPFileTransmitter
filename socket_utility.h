#ifndef SOCKET_UTILITY_H
#define SOCKET_UTILITY_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdio.h>
#include <WinSock2.h>

typedef unsigned short u16;
typedef unsigned long long u64;

typedef struct sockaddr SockAddr;
typedef struct sockaddr_in SockAddr_in;

u64 getMicroCounter();
void printError(char* message);
void sockError(int bQuit);
void sockSetAddress(SockAddr_in* pSockAddr, char *pstrIP, u16 nPort);

u64 getMicroCounter() {
	u64 Counter;

#if defined(_WIN32)
	u64 Frequency;
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER *)&Counter);
	Counter = 1000000 * Counter / Frequency;
#elif defined(__linux__) 
	struct timeval t;
	gettimeofday(&t, 0);
	Counter = 1000000 * t.tv_sec + t.tv_usec;
#endif

	return Counter;
}

void printError(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	sockError(1);
}

void sockError(int bQuit) {
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, 
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error",
		MB_OK | MB_ICONINFORMATION);

	LocalFree(lpMsgBuf);

	if (bQuit) exit(1);
}

void sockSetAddress(SockAddr_in* pSockAddr, char *pstrIP, u16 nPort) {
	if (!pSockAddr) return;

	// memset(pSockAddr, 0, sizeof(*pSockAddr));

	pSockAddr->sin_family = AF_INET;
	pSockAddr->sin_port = htons(nPort);

	if (pstrIP) {
		if (isdigit(pstrIP[0])) {
			pSockAddr->sin_addr.s_addr = inet_addr(pstrIP);
		} else {
			HOSTENT* pHostent;
			if (pHostent = gethostbyname(pstrIP)) {
				memcpy(&pSockAddr->sin_addr.s_addr, pHostent->h_addr, pHostent->h_length);
			} else {
				pSockAddr = NULL;
			}
		}
	} else {
		pSockAddr->sin_addr.s_addr = htonl(INADDR_ANY);
	}
}

#endif