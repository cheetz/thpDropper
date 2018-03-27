#include "networking.h"
#include <vector>
#include <iostream>
#include <errno.h>
#include <malloc.h>
#include <thread>
#include <arpa/inet.h>
void * handlers[PACKET_TYPE::PACKET_TYPE_MAX];
std::vector<std::thread> threads;

void RegisterHandler(PACKET_TYPE type, void * handler)
{
	if(type >= PACKET_TYPE_MAX)
	{
		printf("[!] Invalid packet type [0x%x] attempted to be registered\n", type);
		return;
	}

	if(handlers[type] != NULL)
	{
		printf("[+] Handler already registered for [0x%x], overwriting\n", type);
	}
	handlers[type] = handler;
}

bool ValidateHeader(HEAD_PACKET * packet)
{
	if(packet->signature != PACKET_SIGNATURE)
	{
		printf("[+] Packet signature [%x] did not match global signature [%x]\n", packet->signature, PACKET_SIGNATURE);
		return false;
	}
	if(packet->msg >= PACKET_TYPE::PACKET_TYPE_MAX)
	{
		printf("[+] Packet message type [%d] outside max range [%d]\n",packet->msg, PACKET_TYPE::PACKET_TYPE_MAX);
		return false;
	}
	return true;
}

void HandlerLoop(int conn)
{
	while(true)
	{
		HEAD_PACKET head{};
		int bytesRead = recv(conn, (char *)&head, sizeof(head), MSG_WAITALL | MSG_PEEK);
		if(bytesRead != SOCKET_ERROR)
		{
			if(ValidateHeader(&head))
			{
				callback c = (callback)handlers[head.msg];
				c(conn);				
			}
			else
			{
				return;
			}
		}
		else
		{
			if(bytesRead == SOCKET_ERROR)
			{
				if(errno == ECONNABORTED || errno == ECONNRESET)
				{
					return;
				}
			}			
		}

	}
}

void StartListening(int socket)
{
	if(listen(socket, 500) == 0)
	{
		while(true)
		{
			sockaddr_in client;
			socklen_t clientlen = sizeof(sockaddr_in);
			int* conn = (int *)malloc(sizeof(int));
			*conn = accept(socket, (sockaddr*)&client, &clientlen);
			if(*conn != INVALID_SOCKET)
			{
				char ip[16];
				unsigned int port;
				inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
				port = ntohs(client.sin_port);
				printf("[+] New connection from %s:%d\n", ip, port); 
				std::thread t1(HandlerLoop, *conn);
			       	threads.push_back(std::move(t1));
			}
		}
	}
}

bool BuildAndSend(int conn, PACKET_TYPE msgType, HEAD_PACKET * packet, size_t packetsize)
{
	packet->msg = msgType;
	packet->signature = PACKET_SIGNATURE;
	packet->size = packetsize;

	int bytesRead = send(conn, (char *) packet, packetsize, 0);
	if(bytesRead == SOCKET_ERROR)
	{
		printf("[!] Failed to send [0x%x] bytes, error code [0x%x]\n", packetsize, errno);
		return false;
	}
	return true;
}

int StartupNetworking(int port)
{
	int soc;
	sockaddr_in saddr;
	if((soc = socket(AF_INET, SOCK_STREAM, 0)) != SOCKET_ERROR)
	{
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = INADDR_ANY;
		saddr.sin_port = htons(port);
		if(!bind(soc, (sockaddr*)&saddr, sizeof(saddr)))
		{
			return soc;
		}
		printf("[!] Unable to bind to port [%d] [%d]\n", port, errno);
		return INVALID_SOCKET;
	}
	printf("Unable to create socket\n");
	return INVALID_SOCKET;
}
