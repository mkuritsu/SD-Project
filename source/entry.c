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
#include "entry.h"

struct entry_t *entry_create(char *key, struct data_t *data)
{
    if (key == NULL || data == NULL)
        return NULL;
    struct entry_t *entry = (struct entry_t*) malloc(sizeof(struct entry_t));
    if (entry == NULL)
        return NULL;
    entry->key = key;
    entry->value = data;
    return entry;
}

int entry_destroy(struct entry_t *entry)
{
    if (entry == NULL || entry->key == NULL || entry->value == NULL)
        return -1;
    if (data_destroy(entry->value) == -1)
        return -1;
    free(entry->key);
    free(entry);
    return 0;
}

struct entry_t *entry_dup(struct entry_t *entry)
{
    if (entry == NULL || entry->value == NULL || entry->key == NULL)
        return NULL;
    struct data_t *dataDup = data_dup(entry->value);
    if (dataDup == NULL)
        return NULL;
    char *keyDup = strdup(entry->key);
    if (keyDup == NULL)
    {
        data_destroy(dataDup);
        return NULL;
    }
    struct entry_t *entryDup = entry_create(keyDup, dataDup);
    if (entryDup == NULL)
    {
        data_destroy(dataDup);
        free(keyDup);
        return NULL;
    }
    return entryDup;
}

int entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value)
{
    if (entry == NULL || entry->key == NULL || entry->value == NULL || new_key == NULL || new_value == NULL || data_destroy(entry->value) == -1)
        return -1;
    free(entry->key);
    entry->key = new_key;
    entry->value = new_value;
    return 0;
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2)
{
    if (entry1 == NULL || entry2 == NULL || entry1->key == NULL || entry2->key == NULL)
        return -2;
    int compare = strcmp(entry1->key, entry2->key);
    return compare == 0 ? 0 : compare > 0 ? 1 : -1;
}