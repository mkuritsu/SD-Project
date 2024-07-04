/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "table_skel.h"
#include "table.h"
#include "data.h"
#include "sdmessage.pb-c.h"
#include "table_skel-private.h"
#include "synchronization.h"
#include "zoo_server.h"
#include "client_stub.h"

struct table_t *table_skel_init(int n_lists)
{
    return table_create(n_lists);
}

int table_skel_destroy(struct table_t *table)
{
    return table_destroy(table);
}

int handle_put(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync)
{
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_ENTRY || msg->entry == NULL)
        return -1;
    EntryT *entry = msg->entry;
    struct data_t data;
    data.datasize = entry->value.len;
    data.data = entry->value.data;
    struct data_t *rdata = data_dup(&data);
    if (rdata == NULL)
        return -1;
    struct entry_t *rentry = entry_create(strdup(entry->key), rdata);
    if (rentry == NULL)
    {
        data_destroy(rdata);
        return -1;
    }
    begin_table_write(sync);
    if (table_put(table, entry->key, &data) == -1)
    {
        entry_destroy(rentry);
        end_table_write(sync);
        return -1;
    }
    int32_t sent;
    struct rtable_t *succ;
    do
    {
        succ = zoo_server_get_successor();
        if (succ != NULL)
            sent = rtable_put(succ, rentry);
    } while(succ != NULL && sent == -1);
    end_table_write(sync);
    entry_destroy(rentry);
    printf("--> TABLE PUT\n    > key: %s\n", entry->key);
    printf("    > data(size=%zu) hex: [", entry->value.len);
    for (size_t i = 0; i < entry->value.len; i++)
    {
        printf("%X", entry->value.data[i]);
        if (i != entry->value.len - 1)
            printf(", ");
    }
    printf("]\n");

    printf("    > string representation: \"");
    for (size_t i = 0; i < entry->value.len; i++)
    {
        printf("%c", entry->value.data[i]);
    }
    printf("\"\n");
    msg->opcode += 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    return 0;
}

int handle_get(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync)
{
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY || msg->key == NULL)
        return -1;
    begin_table_read(sync);
    struct data_t *dataGet = table_get(table, msg->key);
    end_table_read(sync);
    if (dataGet == NULL)
        return -1;
    printf("--> TABLE GET:\n    > key: %s\n", msg->key);
    ProtobufCBinaryData binaryData;
    binaryData.data = dataGet->data;
    binaryData.len = dataGet->datasize;
    msg->opcode += 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
    msg->value = binaryData;
    free(dataGet);
    return 0;
}

int handle_del(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync)
{
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_KEY || msg->key == NULL)
        return -1;
    begin_table_write(sync);
    if (table_remove(table, msg->key) != 0)
    {
        end_table_write(sync);
        return -1;
    }
    int32_t sent;
    struct rtable_t *succ;
    do
    {
        succ = zoo_server_get_successor();
        if (succ != NULL)
            sent = rtable_del(succ, msg->key);
    } while(succ != NULL && sent == -1);
    end_table_write(sync);
    printf("--> TABLE DEL:\n    > key: %s\n", msg->key);
    msg->opcode += 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    return 0;
}

int handle_size(MessageT *msg, struct table_t *table)
{
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return -1;
    int size = table_size(table);
    if (size == -1)
        return -1;
    puts("--> TABLE SIZE");
    msg->opcode += 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
    msg->result = size;
    return 0;
}

int handle_getkeys(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync)
{
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return -1;
    begin_table_read(sync);
    int size = table_size(table);
    char **keys = table_get_keys(table);
    end_table_read(sync);
    if (size == -1 || keys == NULL)
        return -1;
    puts("--> TABLE GETKEYS");
    msg->keys = keys;
    msg->n_keys = size;
    msg->opcode += 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
    return 0;
}

int handle_gettable(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync)
{
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return -1;
    begin_table_read(sync);
    int size = table_size(table);
    char **keys = table_get_keys(table);
    if (size == -1 || keys == NULL)
    {
        end_table_read(sync);
        return -1;
    }
    EntryT **entries = (EntryT **)calloc(size, sizeof(EntryT *));
    if (entries == NULL)
    {
        end_table_read(sync);
        table_free_keys(keys);
        return -1;
    }
    for (int i = 0; i < size; i++)
    {
        entries[i] = (EntryT *)malloc(sizeof(EntryT));
        struct data_t *data = table_get(table, keys[i]);
        if (entries[i] == NULL || data == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free(entries[i]->key);
                free(entries[i]->value.data);
                free(entries[i]);
            }
            end_table_read(sync);
            free(entries);
            table_free_keys(keys);
            return -1;
        }
        entry_t__init(entries[i]);
        ProtobufCBinaryData binaryData;
        binaryData.data = data->data;
        binaryData.len = data->datasize;
        entries[i]->key = keys[i];
        entries[i]->value = binaryData;
        free(data);
    }
    end_table_read(sync);
    puts("--> TABLE GETTABLE");
    msg->entries = entries;
    msg->n_entries = size;
    msg->opcode += 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
    free(keys);
    return 0;
}

int handle_stats(MessageT *msg, struct statistics_t *stats, struct table_synchronize_t *sync)
{
    if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        return -1;
    msg->opcode += 1;
    msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;
    msg->statistics = (StatisticsT *)malloc(sizeof(StatisticsT));
    statistics_t__init(msg->statistics);
    begin_stats_read(sync);
    msg->statistics->clients_connected = stats->clients_connected;
    msg->statistics->execution_time = stats->execution_time;
    msg->statistics->num_operations = stats->num_operations;
    end_stats_read(sync);
    puts("--> TABLE STATS");
    return 0;
}

int invoke(MessageT *msg, struct table_t *table, struct statistics_t *stats, struct table_synchronize_t *sync)
{
    if (msg == NULL || table == NULL)
        return -1;
    int result = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_PUT:
        result = handle_put(msg, table, sync);
        break;
    case MESSAGE_T__OPCODE__OP_GET:
        result = handle_get(msg, table, sync);
        break;
    case MESSAGE_T__OPCODE__OP_DEL:
        result = handle_del(msg, table, sync);
        break;
    case MESSAGE_T__OPCODE__OP_SIZE:
        result = handle_size(msg, table);
        break;
    case MESSAGE_T__OPCODE__OP_GETKEYS:
        result = handle_getkeys(msg, table, sync);
        break;
    case MESSAGE_T__OPCODE__OP_GETTABLE:
        result = handle_gettable(msg, table, sync);
        break;
    case MESSAGE_T__OPCODE__OP_STATS:
        result = handle_stats(msg, stats, sync);
        break;
    default:
        result = -1;
        break;
    }
    struct timeval end;
    gettimeofday(&end, NULL);
    if (msg->opcode != MESSAGE_T__OPCODE__OP_STATS + 1)
    {
        suseconds_t micro = end.tv_usec - start.tv_usec;
        suseconds_t seconds = end.tv_sec - start.tv_sec;
        suseconds_t total = seconds*1000000 + micro;
        begin_stats_write(sync);
        stats->execution_time += total;
        stats->num_operations++;
        end_stats_write(sync);
    }

    if (result == -1)
    {
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        return 0;
    }
    puts("");
    return 0;
}
