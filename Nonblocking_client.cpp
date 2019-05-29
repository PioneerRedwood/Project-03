// Windows socket 프로그래밍 클라이언트 코드 <다중 연결>
#define WIN32_LEAN_AND_MEAN

#include<iostream>
#include<string>
#include<ws2tcpip.h>
#include<winsock2.h>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT "9000"

void shoutError(string errormsg)
{
	cout << errormsg << endl;
	WSACleanup();
	exit(1);
}

void shoutError(addrinfo* res, SOCKET sock, string errormsg)
{
	cout << errormsg << endl;
	freeaddrinfo(res);
	closesocket(sock);
	WSACleanup();
	exit(1);
}

int main()
{
	WSADATA wsaData;
	int iResult;
	char buf[DEFAULT_BUFLEN];
	int bufLen = DEFAULT_BUFLEN;

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult == INVALID_SOCKET)
		shoutError("WSAStartup() failed!\n");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// 서버 주소와 포트를 설정
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
		shoutError("getaddrinfo() failed!\n");

	SOCKET ConnectSocket = INVALID_SOCKET;
	ptr = result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET)
		shoutError(result, ConnectSocket, "socket error!\n");

	// 서버에 연결
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
		shoutError(result, ConnectSocket, "connet() error!\n");
	
	int count = 0;
	unsigned un = 0;

	// 서버에 전송
	while (count < 5)
	{
		ZeroMemory(buf, bufLen);
		cout << ">> ";
		cin.getline(buf, bufLen);

		for (int i = 0; i < bufLen; i++)
			if (buf[i] != '\0')
				un++;

		iResult = send(ConnectSocket, buf, un, 0);
		if (iResult == 0)
		{
			cout << "Send failed with error: " << WSAGetLastError() << endl;
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		else
		{
			cout << "#0" << count + 1 << "  Sent: " << iResult << " byte (s)\n";
			iResult = 0;
			count++;
		}
	}
	// 서버로부터 오는 다른 클라이언트의 대화를 받아오기

	// 더이상 전송할 데이터가 없을 때 소켓 닫기
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "shutdown failed: " << WSAGetLastError() << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// 종료
	closesocket(ConnectSocket);
	WSACleanup();
	return 0;
}