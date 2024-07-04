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
#include <stdlib.h>
#include <stdio.h>
#include "client_stub-private.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "message-private.h"

int network_connect(struct rtable_t *rtable)
{
    if(rtable == NULL)
        return -1;
    rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(rtable->sockfd == -1)
        return -1;
    struct sockaddr_in sokaddr;
    sokaddr.sin_family = AF_INET;
    sokaddr.sin_port = htons(rtable->server_port);
    if (inet_pton(AF_INET, rtable->server_address, &sokaddr.sin_addr) < 1)
    {
        close(rtable->sockfd);
        return -1;
    }
    if(connect(rtable->sockfd, (struct sockaddr *)&sokaddr, sizeof(sokaddr)) == -1)
    {
        close(rtable->sockfd);
        return -1;
    }
    return 0;
}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg)
{
    if(rtable == NULL || msg == NULL)
        return NULL;
    int sockfd = rtable->sockfd;
    int size = message_t__get_packed_size(msg);
    uint8_t *buffer = (uint8_t*)malloc(size);
    if(buffer == NULL)
        return NULL;
    message_t__pack(msg, buffer);

    // Send serialized short and msg to server
    uint16_t net_length = htons(size);
    if (write_all(sockfd, &net_length, sizeof(uint16_t)) != sizeof(uint16_t))
    {
        free(buffer);
        return NULL;
    }
    if (write_all(sockfd, buffer, size) != size)
    {
        free(buffer);
        return NULL;
    }
    free(buffer);

    // Receive server response
    uint16_t response_size = 0;
    if (read_all(sockfd, &response_size, sizeof(uint16_t)) != sizeof(uint16_t))
        return NULL;
    response_size = ntohs(response_size);
    uint8_t *receive_buffer = (uint8_t *) malloc(response_size);
    if(receive_buffer == NULL)
        return NULL;
    if (read_all(sockfd, receive_buffer, response_size) != response_size)
    {
        free(receive_buffer);
        return NULL;
    }

    // De-serialize response message
    MessageT* receive_message = message_t__unpack(NULL, response_size, receive_buffer);
    free(receive_buffer);
    return receive_message;
}

int network_close(struct rtable_t *rtable)
{
    if(rtable == NULL || rtable->sockfd < 0)
        return -1;
    return close(rtable->sockfd);
}