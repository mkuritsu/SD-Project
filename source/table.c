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
#include <limits.h>
#include <stdio.h>
#include "table.h"
#include "table-private.h"

struct table_t *table_create(int n)
{
    if (n <= 0)
        return NULL;
    struct table_t *table = (struct table_t*) malloc(sizeof(struct table_t));
    if (table == NULL)
        return NULL;
    table->size = n;
    table->lists = (struct list_t**) calloc(n, sizeof(struct list_t*));
    if (table->lists == NULL)
    {
        free(table);
        return NULL;
    }
    for (int i = 0; i < n; i++)
    {
        table->lists[i] = list_create();
        if (table->lists[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                list_destroy(table->lists[j]);
            }
            free(table->lists);
            free(table);
            return NULL;
        }
    }
    return table;
}

int table_destroy(struct table_t *table)
{
    if (table == NULL)
        return -1;
    for (int i = 0; i < table->size; i++)
    {
        if (list_destroy(table->lists[i]) == -1)
            return -1;
    }
    free(table->lists);
    free(table);
    return 0;
}

int table_put(struct table_t *table, char *key, struct data_t *value)
{
    if (table == NULL || key == NULL || value == NULL)
        return -1;
    int index = hash_code(key, table->size);
    if (index == -1)
        return -1;
    char *keyCopy = strdup(key);
    if (keyCopy == NULL)
        return -1;
    struct data_t *dataCopy = data_dup(value);
    if (dataCopy == NULL)
    {
        free(keyCopy);
        return -1;
    }
    struct entry_t *newEntry = entry_create(keyCopy, dataCopy);
    if (newEntry == NULL)
    {
        free(keyCopy);
        data_destroy(dataCopy);
        return -1;
    }
    if (list_add(table->lists[index], newEntry) == -1)
    {
        entry_destroy(newEntry);
        return -1;
    }
    return 0;
}

struct data_t *table_get(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL)
        return NULL;
    int index = hash_code(key, table->size);
    if (index == -1)
        return NULL;
    struct entry_t *resultGet = list_get(table->lists[index], key);
    if (resultGet == NULL)
        return NULL;
    return data_dup(resultGet->value);
}

int table_remove(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL)
        return -1;
    int index = hash_code(key, table->size);
    if (index == -1)
        return -1;
    return list_remove(table->lists[index], key);
}

int table_size(struct table_t *table)
{
    if (table == NULL)
        return -1;
    int size = 0;
    for (int i = 0; i < table->size; i++)
    {
        int sizePerList = list_size(table->lists[i]);
        if (sizePerList == -1)
            return -1;
        size += sizePerList;
    }
    return size;
}

char **table_get_keys(struct table_t *table)
{
    if (table == NULL)
        return NULL;
    int numKeys = 0;
    for (int i = 0; i < table->size; i++)
    {
        numKeys += list_size(table->lists[i]);
    }
    char **keys = (char **) calloc(numKeys + 1, sizeof(char*));
    if (keys == NULL)
        return NULL;
    int keysBufferIndex = 0;
    for (int i = 0; i < table->size; i++)
    {
        char **listKeys = (char**) list_get_keys(table->lists[i]);
        if (listKeys == NULL)
        {
            for (int j = 0; j < keysBufferIndex; j++)
            {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        for (int j = 0; listKeys[j] != NULL; j++)
        {
            keys[keysBufferIndex] = strdup(listKeys[j]);
            if (keys[keysBufferIndex] == NULL)
            {
                for (int k = 0; k < keysBufferIndex; k++)
                {
                    free(keys[j]);
                }
                free(keys);
                list_free_keys(listKeys);
                return NULL;
            }
            keysBufferIndex++;
        }
        if(list_free_keys(listKeys) == -1)
        {
            for(int k = 0; k < i; k++)
            {
                free(listKeys[k]);
            }
            free(listKeys);
            return NULL;
        }
    }
    return keys;
}

int table_free_keys(char **keys)
{
    if (keys == NULL)
        return -1;
    for (int i = 0; keys[i] != NULL; i++)
    {
        free(keys[i]);
    }
    free(keys);
    return 0;
}

int hash_code(char* key, int n)
{
    if(key == NULL || n <= 0)
        return -1;
    int hash = 7;
    for(int i = 0; i < strlen(key); i++)
    {
        hash += hash * 11 + key[i];
    }
    return (hash & INT_MAX) % n;
}