/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <zookeeper/zookeeper.h>
#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "message-private.h"
#include "network_server.h"
#include "network_server-private.h"
#include "client_stub.h"
#include "zoo_server.h"


int network_server_init(short port)
{
    if (port <= 0)
        return -1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return -1;
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddr.sin_port = htons(port);
    const int reuse_value = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_value, sizeof(int)) == -1)
    {
        close(sockfd);
        return -1;
    }
    if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1)
    {
        close(sockfd);
        return -1;
    }
    if (listen(sockfd, 0) == -1)
    {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

void* client_loop(void *arg)
{
    struct thread_info* thread_info = (struct thread_info*) arg;
    int client_socket = thread_info->client_socket;
    struct table_t *table = thread_info->table;
    struct statistics_t *stats = thread_info->stats;
    struct table_synchronize_t *sync = thread_info->sync;
    free(thread_info);
    begin_stats_write(sync);
    stats->clients_connected++;
    end_stats_write(sync);
    puts("Client connected!");
    while (1)
    {
        MessageT *msg = network_receive(client_socket);
        if (msg == NULL)
            break;
        if (invoke(msg, table, stats, sync) == -1)
        {
            message_t__free_unpacked(msg, NULL);
            break;
        }
        if (network_send(client_socket, msg) == -1)
        {
            message_t__free_unpacked(msg, NULL);
            break;
        }
        message_t__free_unpacked(msg, NULL);
    }
    close(client_socket);
    begin_stats_write(sync);
    stats->clients_connected--;
    end_stats_write(sync);
    puts("Client disconnected or an error ocurred!");
    return 0;
}

int network_main_loop(int listening_socket, struct table_t *table, struct statistics_t* stats, struct table_synchronize_t *sync)
{
    if (listening_socket < 0 || table == NULL || stats == NULL || sync == NULL)
        return -1;
    int client_socket = 0;
    struct sockaddr_in client_addr;
    socklen_t client_size = 0;
    do
    {
        client_socket = accept(listening_socket, (struct sockaddr *)&client_addr, &client_size);
        if (client_socket != -1)
        {
            pthread_t thread;
            struct thread_info* thread_info = (struct thread_info*) malloc(sizeof(struct thread_info));
            if(thread_info == NULL)
            {
                close(client_socket);
                continue;
            }
            thread_info->client_socket = client_socket;
            thread_info->table = table;
            thread_info->stats = stats;
            thread_info->sync = sync;
            if (pthread_create(&thread, NULL, client_loop, (void *) thread_info) != 0)
            {
                free(thread_info);
                fprintf(stderr,"(!) Error: Failed to create thread.\n");
                break;
            }
            if(pthread_detach(thread) != 0){
                fprintf(stderr,"(!) Error: Failed to detach thread.\n");
                break;
            }
        }
    } while (client_socket != -1);
    return -1;
}

MessageT *network_receive(int client_socket)
{
    if (client_socket < 0)
        return NULL;
    uint16_t length = 0;
    if (read_all(client_socket, &length, sizeof(uint16_t)) != sizeof(uint16_t))
        return NULL;
    length = ntohs(length);
    uint8_t *buffer = (uint8_t *)malloc(length);
    if (buffer == NULL)
        return NULL;
    if (read_all(client_socket, buffer, length) != length)
    {
        free(buffer);
        return NULL;
    }
    MessageT *msg = message_t__unpack(NULL, length, buffer);
    free(buffer);
    return msg;
}

int network_send(int client_socket, MessageT *msg)
{
    if (client_socket < 0 || msg == NULL)
        return -1;
    u_int16_t length = (u_int16_t)message_t__get_packed_size(msg);
    uint16_t net_length = htons(length); // short with buffer size
    uint8_t *buffer = (uint8_t *)malloc(length);
    if (buffer == NULL)
        return -1;
    message_t__pack(msg, buffer);
    if (write_all(client_socket, &net_length, sizeof(uint16_t)) != sizeof(uint16_t))
    {
        free(buffer);
        return -1;
    }
    if (write_all(client_socket, buffer, length) != length)
    {
        free(buffer);
        return -1;
    }
    free(buffer);
    return 0;
}

int network_server_close(int socket)
{
    if (socket < 0)
        return -1;
    return close(socket);
}