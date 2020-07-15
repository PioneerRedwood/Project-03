// Web networking with winsock
// https://guidedhacking.com/threads/c-winsock-networking-tutorial-introduction.12131/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#pragma comment(lib, "ws2_32.lib");
#include <WinSock2.h>
#include <cstdlib>
#include <iostream>
#include <Windows.h>

#define PORT 80

// 일부 도메인만 가능함 google, youtube .. 
const char szHost[] = "www.google.com";

int main(const int argc, const char* argv[])
{
	// Init WinSock
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	errno_t error;
	if ((error = WSAStartup(DllVersion, &wsaData)) != 0)
	{
		// 예외 처리
		ExitProcess(EXIT_FAILURE);
	}
		

	// Create Socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		// 예외 처리
		ExitProcess(EXIT_FAILURE);
	}


	// Get server info
	HOSTENT* host = gethostbyname(szHost);
	if (host == nullptr)
	{
		// 예외 처리
		ExitProcess(EXIT_FAILURE);
	}

	// Define server info
	SOCKADDR_IN sin;
	ZeroMemory(&sin, sizeof(sin));
	sin.sin_port = htons(PORT);
	sin.sin_family = AF_INET;
	memcpy_s(&sin.sin_addr.S_un.S_addr, sizeof(sin.sin_addr.S_un.S_addr) - 1, 
		host->h_addr_list[0], sizeof(sin.sin_addr.S_un.S_addr) - 1);

	// Connect to server
	if (connect(sock, (const sockaddr*)&sin, sizeof(sin)) != 0)
	{
		// 예외 처리
		ExitProcess(EXIT_FAILURE);
	}

	// Send to server
	const char* szMsg = "HEAD / HTTP/1.0\r\n\r\n";
	if (!send(sock, szMsg, std::strlen(szMsg), 0))
	{
		// 예외 처리
		ExitProcess(EXIT_FAILURE);
	}


	// Receive data back from server
	char szBuffer[2048];
	ZeroMemory(szBuffer, 2048);

	while (recv(sock, szBuffer, 2048, 0))
	{
		// 받아 온 것 전시
		std::cout << "get in recv()" << std::endl;
		std::cout << szBuffer << std::endl;
	}

	closesocket(sock);
	ExitProcess(EXIT_FAILURE);
}