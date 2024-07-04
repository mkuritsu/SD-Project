/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "client_stub.h"
#include "zoo_server.h"

static struct rtable_t *s_successor = NULL;
static char s_successor_path[ZOO_PATH_LEN];
static char s_self_path[ZOO_PATH_LEN];


// Função watcher chamada cada vez que ocorre alguma alteração nos filhos para reconfigurar as ligações entre os servidores na chain.
static void child_watcher(zhandle_t *zh, int type, int state, const char *path, void *context)
{
    if (state == ZOO_CONNECTED_STATE && type == ZOO_CHILD_EVENT)
    {
        struct String_vector children;
        if (zoo_wget_children(zh, RTABLE_ZOO_ROOT, child_watcher, NULL, &children) != ZOK)
        {
            fprintf(stderr, "(!) Error: Failed to get childrens!\n");
            raise(SIGINT);
            // exit(EXIT_FAILURE);
        }
        char next_path[ZOO_PATH_LEN] = "";
        for (int32_t i = 0; i < children.count; i++)
        {
            char *node_path = children.data[i];
            char full_path[strlen(RTABLE_ZOO_ROOT) + strlen(node_path) + 2];
            sprintf(full_path, "%s/%s", RTABLE_ZOO_ROOT, node_path);
            if (strcmp(next_path, "") == 0 && strcmp(full_path, s_self_path) > 0)
                strcpy(next_path, full_path);
            else if (strcmp(next_path, "") != 0 && strcmp(full_path, next_path) < 0 && strcmp(full_path, s_self_path) > 0)
                strcpy(next_path, full_path);
        }
        if (strcmp(next_path, "") == 0)
        {
            if (s_successor != NULL)
            {
                rtable_disconnect(s_successor);
                s_successor = NULL;
            }
            return;
        }
        else if (strcmp(next_path, s_successor_path) != 0)
        {
            if (connect_to_rtable(zh, next_path, &s_successor) == -1)
            {
                fprintf(stderr, "(!) Error: Failed to connect to next server!\n");
                raise(SIGINT);
                // exit(EXIT_FAILURE);
            }
            printf(" > CONNECTED TO SUCCESSOR %s\n", next_path);
            strcpy(s_successor_path, next_path);
        }
    }
}

// Função de watcher que é chamada cada vez que se tenta estabelecer uma conexão com o Zookeeper.
static void connection_watcher(zhandle_t *zh, int type, int state, const char *path, void *context)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            printf("Connected to ZooKeeper!\n");
        }
        else
        {
            fprintf(stderr, "(!) Error: Couldn't connect to ZooKeeper!");
            raise(SIGINT);
            // exit(EXIT_FAILURE);
        }
    }
}

// Função que copia a tabela do servidor com o maior ip que seja menor que o do servidor que acabou de ser criado, e realiza a cópia da tabela.
static int32_t zoo_server_retrieve_table(zhandle_t *zh, struct table_t *table)
{
    struct String_vector children;
    if (zoo_wget_children(zh, RTABLE_ZOO_ROOT, child_watcher, NULL, &children) != ZOK)
        return -1;
    if (children.count == 1)
        return 0;
    char prev_path[ZOO_PATH_LEN] = "";
    for (int32_t i = 0; i < children.count; i++)
    {
        char *node_path = children.data[i];
        char full_path[strlen(RTABLE_ZOO_ROOT) + strlen(node_path) + 2];
        sprintf(full_path, "%s/%s", RTABLE_ZOO_ROOT, node_path);
        if (strcmp(prev_path, "") == 0 && strcmp(full_path, s_self_path) < 0)
            strcpy(prev_path, full_path);
        else if (strcmp(prev_path, "") != 0 && strcmp(full_path, prev_path) > 0 && strcmp(full_path, s_self_path) < 0)
            strcpy(prev_path, full_path);
    }
    struct rtable_t *rtable = NULL;
    if (connect_to_rtable(zh, prev_path, &rtable) == -1)
        return -1;
    struct entry_t **entries = rtable_get_table(rtable);
    if (entries == NULL)
    {
        rtable_disconnect(rtable);
        return -1;
    }
    for (int32_t i = 0; entries[i] != NULL; i++)
    {
        if (table_put(table, entries[i]->key, entries[i]->value) == -1)
        {
            rtable_free_entries(entries);
            rtable_disconnect(rtable);
            return -1;
        }
    }
    rtable_disconnect(rtable);
    rtable_free_entries(entries);
    return 0;
}

// Função que cria o nó do servidor no Zookeeper. Retorna 0, ou -1 em caso de erro.
static int32_t create_server_node(zhandle_t *zh, const char *ip_addr, const int16_t port)
{
    char node_data[strlen(ip_addr) + 7]; // ip + : + max 5 chars port (int16_t max) + \0 = strlen(ip) + 7
    memset(node_data, 0, sizeof(node_data));
    sprintf(node_data, "%s:%d", ip_addr, port);
    if (zoo_create(zh, RTABLE_NODE_PATH, node_data, sizeof(node_data), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, s_self_path, sizeof(s_self_path)) != ZOK)
        return -1;
    printf(" > CREATED SERVER NODE %s WITH DATA %s\n", s_self_path, node_data);
    return 0;
}


zhandle_t *zoo_server_init(const char *host, const char *ip_addr, const int16_t port, struct table_t *table)
{
    zhandle_t *zh = zookeeper_init(host, connection_watcher, 2000, NULL, NULL, 0);
    if (zh == NULL)
        return NULL;
    if (zoo_exists(zh, RTABLE_ZOO_ROOT, 0, NULL) == ZNONODE)
    {
        if (zoo_create(zh, RTABLE_ZOO_ROOT, NULL, -1, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0) != ZOK)
            return NULL;
    }
    if (create_server_node(zh, ip_addr, port) == -1)
    {
        zookeeper_close(zh);
        return NULL;
    }
    if (zoo_server_retrieve_table(zh, table) == -1)
    {
        zookeeper_close(zh);
        return NULL;
    }
    return zh;
}

void zoo_server_close()
{
    if (s_successor != NULL)
        rtable_disconnect(s_successor);
}

struct rtable_t *zoo_server_get_successor()
{
    return s_successor;
}