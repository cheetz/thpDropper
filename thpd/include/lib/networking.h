#include <sys/types.h>
#include <sys/socket.h>
#include <cstdint>
#include <netinet/in.h>
#include <netinet/ip.h>
#define INVALID_SOCKET (~0)
#define SOCKET_ERROR -1
#define PACKET_SIGNATURE 0x1337

enum PACKET_TYPE
{
	LOAD_BLOB,
	PACKET_TYPE_MAX,
};

typedef struct _head_packet
{
	uint32_t signature;
	PACKET_TYPE msg;
	uint32_t size;
} HEAD_PACKET, *PHEAD_PACKET;

typedef void (*callback)(int conn);

void RegisterHandler(PACKET_TYPE type, void * handler);
int StartupNetworking(int);
bool BuildAndSend(int, PACKET_TYPE, HEAD_PACKET *, size_t);
void StartListening(int);
