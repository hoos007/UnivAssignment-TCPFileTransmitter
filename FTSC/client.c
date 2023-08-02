#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include "../socket_utility.h"

#define APP_RCV_BUF_SIZE 65495

int main(int argc, char* argv[]) {
	WSADATA wsa;
	struct sockaddr_in server_addr;
	SOCKET s;
	FILE* fp;
	u64 start, end;

	int on = 1;
	int sockoptstate;


	// Winsock Initialization
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) sockError(1);

	// Open a Socket
	if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) sockError(1);

	// Set peer/server address and CONNECT
	sockSetAddress(&server_addr, argv[1], atoi(argv[2]));
	if (connect(s, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		sockError(1);

	sockoptstate = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(on));
	if (sockoptstate)
	{
		printf("소켓 옵션 설정 에러");
		exit(1);
	}


	puts("====TCP File Transmitter Client====");
	{ // Main Operation Block
		char buf[APP_RCV_BUF_SIZE];
		int flag = 0;

		int fsize;
		int totalReceiveSize = 0;
		int receiveSize;
		int timerFlag = 0;
		int time;

		fp = fopen(argv[3], "wb");
		if (fp == NULL)
		{
			printf("파일 열기 실패 종료합니다.");
			exit(1);
		}

		do {
			receiveSize = recv(s, buf, APP_RCV_BUF_SIZE, 0);
			if (receiveSize > 0)
			{
				if (timerFlag == 0)
				{
					start = getMicroCounter();
					timerFlag = 1;
				}
				fwrite(buf, sizeof(char), receiveSize, fp);
				totalReceiveSize += receiveSize;
				printf("Received File : [%d]\n", totalReceiveSize);

			}
			if (receiveSize == SOCKET_ERROR) {
				flag = 1;
				sockError(0);
				break;
			}
		} while (receiveSize > 0);

		if (flag == 0)
		{
			end = getMicroCounter();
			time = end - start;
			printf("수신 성공\n");
			printf("소요시간(micro seconds) : %d us, 전송속도(bps) : %.2f bps\n", time, (totalReceiveSize) / (double)(time / 1000000));
		}
		else
		{
			printf("수신 실패\n");
		}
	}

	// Close the opened socket
	closesocket(s);
	// Winsock Finalization
	WSACleanup();
	return 0;
}