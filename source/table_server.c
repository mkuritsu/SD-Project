/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include "network_server.h"
#include "table_skel.h"
#include "synchronization.h"
#include "client_stub.h"
#include "zoo_server.h"

static struct table_t *s_table_t = NULL;
static struct table_synchronize_t *s_sync = NULL;
static int s_sockfd = -1;
static zhandle_t *s_zh = NULL;

static void ctrlC(int sig)
{
    signal(SIGINT, ctrlC);
    table_skel_destroy(s_table_t);
    network_server_close(s_sockfd);
    synchronize_destroy(s_sync);
    zoo_server_close();
    zookeeper_close(s_zh);
    exit(EXIT_SUCCESS);
}

static const char *get_ip_addr()
{
    struct ifaddrs *ifap = NULL;
    struct sockaddr_in sock_addr = {0};
    socklen_t addr_len = {0};
    getsockname(s_sockfd, (struct sockaddr *)&sock_addr, &addr_len);
    char *addr = NULL;
    getifaddrs(&ifap);
    for (struct ifaddrs *ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            if (strcmp(addr, "127.0.0.1") != 0)
                break;
        }
    }
    freeifaddrs(ifap);
    return addr;
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, ctrlC);
    if (argc != 4)
    {
        char *binaryName;
        if (argc > 0)
            binaryName = argv[0];
        else
            binaryName = "./table_server";
        printf("Usage: %s <port> <table size> <zookeeper host>\n", binaryName);
        printf("e.g: %s 25565 10 localhost:2181\n", binaryName);
        exit(EXIT_FAILURE);
    }
    uint16_t port = (uint16_t)atoi(argv[1]);
    if (port <= 0)
    {
        fprintf(stderr, "(!) Error: Invalid port number (%d)!\n", port);
        exit(EXIT_FAILURE);
    }
    int tableSize = atoi(argv[2]);
    s_table_t = table_skel_init(tableSize);
    if (s_table_t == NULL)
    {
        fprintf(stderr, "(!) Error: Failed to create hash table!");
        exit(EXIT_FAILURE);
    }
    s_sockfd = network_server_init(port);
    if (s_sockfd == -1)
    {
        fprintf(stderr, "(!) Error: Failed to initialize network server\n!");
        table_skel_destroy(s_table_t);
        exit(EXIT_FAILURE);
    }
    const char *ip_addr = get_ip_addr();
    if (ip_addr == NULL)
    {
        table_skel_destroy(s_table_t);
        network_server_close(s_sockfd);
        fprintf(stderr, "(!) Error: Failed to get ip address\n");
        exit(EXIT_FAILURE);
    }
    zhandle_t *s_zh = zoo_server_init(argv[3], ip_addr, port, s_table_t);
    if (s_zh == NULL)
    {
        table_skel_destroy(s_table_t);
        network_server_close(s_sockfd);
        fprintf(stderr, "(!) Error: Failed to connect to ZooKeeper!\n");
        exit(EXIT_FAILURE);
    }
    s_sync = synchronize_create();
    if (s_sync == NULL)
    {
        fprintf(stderr, "(!) Error: Failed to initialize network server\n!");
        table_skel_destroy(s_table_t);
        network_server_close(s_sockfd);
        zookeeper_close(s_zh);
        exit(EXIT_FAILURE);
    }
    struct statistics_t stats = {0};
    if (network_main_loop(s_sockfd, s_table_t, &stats, s_sync) == -1)
    {
        table_skel_destroy(s_table_t);
        network_server_close(s_sockfd);
        synchronize_destroy(s_sync);
        zookeeper_close(s_zh);
        fprintf(stderr, "(!) Error: Failed to run network server main loop!\n");
        exit(EXIT_FAILURE);
    }
    synchronize_destroy(s_sync);
    if (network_server_close(s_sockfd) == -1)
    {
        table_skel_destroy(s_table_t);
        zookeeper_close(s_zh);
        fprintf(stderr, "(!) Error: Failed to close network socket!");
        exit(EXIT_FAILURE);
    }
    if (table_skel_destroy(s_table_t) == -1)
    {
        zookeeper_close(s_zh);
        fprintf(stderr, "(!) Error: Failed to destroy table skel!");
        exit(EXIT_FAILURE);
    }
    zookeeper_close(s_zh);
    return 0;
}
