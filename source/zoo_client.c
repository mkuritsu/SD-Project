/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "zoo_client.h"

static struct rtable_t *s_write_table = NULL;
static struct rtable_t *s_read_table = NULL;
static char s_head_node[ZOO_PATH_LEN] = "";
static char s_tail_node[ZOO_PATH_LEN] = "";

static void child_watcher(zhandle_t *zh, int type, int state, const char *path, void *context);

// Função que realiza a ligação dos clientes aos servidores head e tail.
static int32_t connect_to_servers(zhandle_t *zh)
{
    struct String_vector children;
    if (zoo_wget_children(zh, RTABLE_ZOO_ROOT, child_watcher, NULL, &children) != ZOK)
        return -1;
    if (children.count == 0)
        return -1;
    char first_path[ZOO_PATH_LEN];
    char last_path[ZOO_PATH_LEN];
    sprintf(first_path, "%s/%s", RTABLE_ZOO_ROOT, children.data[0]);
    sprintf(last_path, "%s/%s", RTABLE_ZOO_ROOT, children.data[0]);
    for (int32_t i = 1; i < children.count; i++)
    {
        char *node_path = children.data[i];
        char full_path[strlen(RTABLE_ZOO_ROOT) + strlen(node_path) + 2];
        sprintf(full_path, "%s/%s", RTABLE_ZOO_ROOT, node_path);
        if (strcmp(full_path, last_path) > 0)
            strcpy(last_path, full_path);
        if (strcmp(full_path, first_path) < 0)
            strcpy(first_path, full_path);
    }
    if (strcmp(first_path, s_head_node) != 0)
    {
        if (connect_to_rtable(zh, first_path, &s_write_table) == -1)
            return -1;
        printf("\r> CONNECTED TO HEAD NODE: %s\n\n$> ", first_path);
        fflush(stdout);
        strcpy(s_head_node, first_path);
    }
    if (strcmp(last_path, s_tail_node) != 0)
    {
        if (connect_to_rtable(zh, last_path, &s_read_table) == -1)
            return -1;
        printf("\r> CONNECTED TO TAIL NODE: %s\n\n$> ", last_path);
        fflush(stdout);
        strcpy(s_tail_node, last_path);
    }
    return 0;
}

// Função de watcher chamada cada vez que ocorre alguma alteração nos filhos para reconfigurar as ligações dos clients aos servidores.
static void child_watcher(zhandle_t *zh, int type, int state, const char *path, void *context)
{
    if (state == ZOO_CONNECTED_STATE && type == ZOO_CHILD_EVENT)
    {
        if (connect_to_servers(zh) == -1)
        {
            fprintf(stderr, "Failed to connect to servers!\n");
            raise(SIGINT);
            // exit(EXIT_FAILURE);
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
            fprintf(stderr, "(!) Error: Couldn't connect to ZooKeeper :(\n");
            raise(SIGINT);
            // exit(EXIT_FAILURE);
        }
    }
}

zhandle_t *zoo_client_init(const char *host)
{
    zhandle_t *zh = zookeeper_init(host, connection_watcher, 2000, NULL, NULL, 0);
    if (zh == NULL)
        return NULL;
    if (connect_to_servers(zh) == -1)
    {
        zookeeper_close(zh);
        fprintf(stderr, "Failed to connect to servers!\n");
        raise(SIGINT);
        // exit(EXIT_FAILURE);
    }
    return zh;
}

void zoo_client_close()
{
    if (s_write_table != NULL)
        rtable_disconnect(s_write_table);
    if (s_read_table != NULL)
        rtable_disconnect(s_read_table);
}

struct rtable_t *zoo_client_get_write_table(void)
{
    return s_write_table;
}

struct rtable_t *zoo_client_get_read_table(void)
{
    return s_read_table;
}