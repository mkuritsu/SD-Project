/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "message-private.h"

int read_all(int sockfd, void *buffer, int size)
{
    int expectedSize = size;
    while (size > 0) 
    {
        int received = read(sockfd, buffer, size);
        if (received < 0) 
        {
            if (errno == EINTR)
                continue;
            return received;
        }
        if (received == 0)
            return received;
        buffer += received;
        size -= received;
    }
    return expectedSize;
}

int write_all(int sockfd, void *buffer, int size)
{
    int bufSize = size;
    while (size > 0)
    {
        int sent = write(sockfd, buffer, size);
        if (sent < 0) {
            if (errno == EINTR)
                continue;
            return sent;
        }
        if (sent == 0)
            return sent;
        buffer += sent;
        size -= sent;
    }
    return bufSize;
}