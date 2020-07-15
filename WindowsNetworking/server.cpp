// NewStart branch 
// Server programming
// 2020.07.15. ~ 

#include <iostream>
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string>
#include <sstream>

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT 65000
#define BACKLOG 10

#pragma comment(lib, "ws2_32.lib")

int main()
{
	using std::cout;
	using std::endl;

	WSADATA wsaData;
	int iResult;

	char buffer[DEFAULT_BUFLEN];
	
	// 소켓 환경 초기화
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		cout << ">> Error at WSAStartup()!\nexit()";
		exit(EXIT_FAILURE);
	}

	sockaddr_in hint;

	// 클라이언트 접속 받을 소켓 생성
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << ">> Error at making a ListenSocket!\nexit()";
		exit(EXIT_FAILURE);
	}

	// 통신 프로토콜 설정
	ZeroMemory(&hint, sizeof(hint));
	hint.sin_family = AF_INET;
	// htons: Host TO Network Short
	hint.sin_port = htons(DEFAULT_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	// TCP 통신 소켓 설정
	iResult = bind(ListenSocket, (sockaddr*)&hint, sizeof(hint));
	if (iResult == SOCKET_ERROR)
	{
		cout << ">> Error at bind()!\nexit()";
		exit(EXIT_FAILURE);
	}

	//// 서버 주소와 포트 번호
	//iResult = getaddrinfo(NULL, DEFAULT_PORT, &hint, &result);
	//if (iResult != 0)
	//{
	//	cout << ">> Error at getaddrinfo()!\nexit()";
	//	exit(EXIT_FAILURE);
	//}	


	// listen 시작
	if (listen(ListenSocket, BACKLOG) == SOCKET_ERROR)
	{
		cout << ">> Error at listen()!\nexit()";
		exit(EXIT_FAILURE);
	}

	cout << ">> 접속을 기다리고 있습니다." << endl;

	fd_set master;
	FD_ZERO(&master);

	FD_SET(ListenSocket, &master);

	while (true)
	{
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == ListenSocket)
			{
				// 기존 연결된 소켓일 경우
				SOCKET client = accept(ListenSocket, nullptr, nullptr);

				// 연결되어 있는 클라이언트 리스트에 추가
				FD_SET(client, &master);

				// 환영 메시지 전달
				std::string welcomeMsg = ">> Welcome! Connected with Server\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

				// 해야할 일: 연결시 서버에도 접속된 클라이언트 정보 보여주기
				
			}
			else
			{
				// 새로운 연결인 경우
				ZeroMemory(buffer, DEFAULT_BUFLEN);
				iResult = recv(sock, buffer, DEFAULT_BUFLEN, 0);

				// 버퍼에 든 값 확인 필요
				std::cout << "SOCKET #" << sock << ": " << buffer;
				if (iResult <= 0)
				{
					// 클라이언트 연결 종료
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// 다른 클라이언트에 전송, 연결된 것 제외
					// 형식이 이상함 수정 필요
					for (unsigned i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock != ListenSocket && outSock != sock)
						{
							std::ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buffer << "\r";
							std::string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	// 종료
	WSACleanup();
	return 0;
}