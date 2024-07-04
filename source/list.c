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
#include "list.h"
#include "list-private.h"

struct list_t *list_create()
{
    struct list_t *list = (struct list_t *)malloc(sizeof(struct list_t));
    if (list == NULL)
        return NULL;
    list->head = NULL;
    list->size = 0;
    return list;
}

int list_destroy(struct list_t *list)
{
    if (list == NULL)
        return -1;
    while (list->head != NULL)
    {
        struct node_t *next = list->head->next;
        if (entry_destroy(list->head->entry) == -1)
            return -1;
        free(list->head);
        list->head = next;
    }
    free(list);
    return 0;
}

int list_add(struct list_t *list, struct entry_t *entry)
{
    if (list == NULL || entry == NULL)
        return -1;
    if (list->head == NULL)
    {
        struct node_t *node = (struct node_t*) malloc(sizeof(struct node_t));
        node->entry = entry;
        node->next = NULL;
        list->head = node;
        list->size++;
        return 0;
    }
    struct node_t *prev = NULL;
    struct node_t *curr = list->head;
    while (curr != NULL)
    {
        int compare = entry_compare(entry, curr->entry);
        if (compare == -2)
            return -1;
        if (compare == 0)
        {
            if (entry_destroy(curr->entry) == -1)
                return -1;
            curr->entry = entry;
            return 1;
        }
        else if (compare == -1)
        {
            struct node_t *node = (struct node_t*) malloc(sizeof(struct node_t));
            if (node == NULL)
                return -1;
            node->entry = entry;
            if (prev == NULL)
            {
                node->next = list->head;
                list->head = node;
            }
            else
            {
                node->next = curr;
                prev->next = node;
            }
            list->size++;
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    struct node_t *node = (struct node_t*) malloc(sizeof(struct node_t));
    node->entry = entry;
    node->next = NULL;
    prev->next = node;
    list->size++;
    return 0;
}


int list_remove(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL)
        return -1;
    if (list->size == 0)
        return 1;
    struct node_t *prev = NULL;
    struct node_t *curr = list->head;
    while (curr != NULL)
    {
        if (strcmp(curr->entry->key, key) == 0)
        {
            if (entry_destroy(curr->entry) == -1)
                return -1;
            if (prev == NULL)
                list->head = curr->next;
            else
                prev->next = curr->next;
            list->size--;
            free(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return 1;
}

struct entry_t *list_get(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL)
        return NULL;
    struct node_t *curr = list->head;
    while (curr != NULL)
    {
        if (strcmp(curr->entry->key, key) == 0)
            return curr->entry;
        curr = curr->next;
    }
    return NULL;
}

int list_size(struct list_t *list)
{
    if (list == NULL)
        return -1;
    return list->size;
}

char **list_get_keys(struct list_t *list)
{
    if (list == NULL)
        return NULL;
    char **arrKeys = (char **)calloc(list->size + 1, sizeof(char *));
    if (arrKeys == NULL)
        return NULL;
    struct node_t *curr = list->head;
    for (int i = 0; i < list->size; i++)
    {
        arrKeys[i] = strdup(curr->entry->key);
        if (arrKeys[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free(arrKeys[j]);
            }
            free(arrKeys);
            return NULL;
        }
        curr = curr->next;
    }
    return arrKeys;
}

int list_free_keys(char **keys)
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