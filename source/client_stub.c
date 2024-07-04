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
#include <stdio.h>
#include "data.h"
#include "entry.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "table.h"

struct rtable_t *rtable_connect(char *address_port)
{
	if (address_port == NULL)
		return NULL;
	char *address = strtok(address_port, ":");
	if (address == NULL)
		return NULL;
	char *port = strtok(NULL, ":");
	if (port == NULL)
		return NULL;
	struct rtable_t *table = (struct rtable_t *)malloc(sizeof(struct rtable_t));
	if (table == NULL)
		return NULL;
	table->server_address = (char *)malloc(strlen(address) + 1);
	if (table->server_address == NULL)
	{
		free(table);
		return NULL;
	}
	strcpy(table->server_address, address);
	table->server_port = atoi(port);
	if (network_connect(table) == -1)
	{
		free(table->server_address);
		free(table);
		return NULL;
	}
	return table;
}

int rtable_disconnect(struct rtable_t *rtable)
{
	if (rtable == NULL)
		return -1;
	int result = network_close(rtable);
	free(rtable->server_address);
	free(rtable);
	return result;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry)
{
	if (rtable == NULL || entry == NULL)
		return -1;
	MessageT request;
	message_t__init(&request);
	request.opcode = MESSAGE_T__OPCODE__OP_PUT;
	request.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;
	request.entry = (EntryT *)malloc(sizeof(EntryT));
	if (request.entry == NULL)
		return -1;
	entry_t__init(request.entry);
	request.entry->key = entry->key;
	request.entry->value.data = entry->value->data;
	request.entry->value.len = entry->value->datasize;
	MessageT *response = network_send_receive(rtable, &request);
	if (response == NULL)
	{
		free(request.entry);
		return -1;
	}
	if (response->opcode != request.opcode + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_NONE)
	{
		free(request.entry);
		message_t__free_unpacked(response, NULL);
		return -1;
	}
	free(request.entry);
	message_t__free_unpacked(response, NULL);
	return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key)
{
	if (rtable == NULL || key == NULL)
		return NULL;
	MessageT request;
	message_t__init(&request);
	request.opcode = MESSAGE_T__OPCODE__OP_GET;
	request.c_type = MESSAGE_T__C_TYPE__CT_KEY;
	request.key = key;
	MessageT *response = network_send_receive(rtable, &request);
	if (response == NULL)
		return NULL;
	if (response->opcode != request.opcode + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_VALUE)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	void *value_data_dup = malloc(response->value.len);
	if (value_data_dup == NULL)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	memcpy(value_data_dup, response->value.data, response->value.len);
	struct data_t *data = data_create(response->value.len, value_data_dup);
	message_t__free_unpacked(response, NULL);
	if (data == NULL)
	{
		free(value_data_dup);
		return NULL;
	}
	return data;
}

int rtable_del(struct rtable_t *rtable, char *key)
{
	if (rtable == NULL || key == NULL)
		return -1;
	MessageT request;
	message_t__init(&request);
	request.opcode = MESSAGE_T__OPCODE__OP_DEL;
	request.c_type = MESSAGE_T__C_TYPE__CT_KEY;
	request.key = key;
	MessageT *response = network_send_receive(rtable, &request);
	if (response->opcode != request.opcode + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_NONE)
	{
		message_t__free_unpacked(response, NULL);
		return -1;
	}
	message_t__free_unpacked(response, NULL);
	return 0;
}

int rtable_size(struct rtable_t *rtable)
{
	if (rtable == NULL)
		return -1;
	MessageT request;
	message_t__init(&request);
	request.opcode = MESSAGE_T__OPCODE__OP_SIZE;
	request.c_type = MESSAGE_T__C_TYPE__CT_NONE;
	MessageT *response = network_send_receive(rtable, &request);
	if (response == NULL)
		return -1;
	if (response->opcode != request.opcode + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_RESULT)
	{
		message_t__free_unpacked(response, NULL);
		return -1;
	}
	int result = response->result;
	message_t__free_unpacked(response, NULL);
	return result;
}

char **rtable_get_keys(struct rtable_t *rtable)
{
	if (rtable == NULL)
		return NULL;
	MessageT request;
	message_t__init(&request);
	request.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
	request.c_type = MESSAGE_T__C_TYPE__CT_NONE;
	MessageT *response = network_send_receive(rtable, &request);
	if (response == NULL)
		return NULL;
	if (response->opcode != request.opcode + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_KEYS)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	char **result = (char **)calloc(response->n_keys + 1, sizeof(char *));
	if (result == NULL)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	for (int i = 0; i < response->n_keys; i++)
	{
		char *keyMsg = response->keys[i];
		result[i] = (char *)malloc(strlen(keyMsg) + 1);
		if (result[i] == NULL)
		{
			for (int j = 0; j < i; j++)
			{
				free(result[j]);
			}
			free(result);
			message_t__free_unpacked(response, NULL);
			return NULL;
		}
		strcpy(result[i], keyMsg);
	}
	message_t__free_unpacked(response, NULL);
	return result;
}

void rtable_free_keys(char **keys)
{
	table_free_keys(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable)
{
	if (rtable == NULL)
		return NULL;
	MessageT request;
	message_t__init(&request);
	request.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
	request.c_type = MESSAGE_T__C_TYPE__CT_NONE;
	MessageT *response = network_send_receive(rtable, &request);
	if (response == NULL)
		return NULL;
	if (response->opcode != request.opcode + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_TABLE)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	EntryT **entries = response->entries;
	struct entry_t **result = (struct entry_t **)calloc(response->n_entries + 1, sizeof(struct entry_t *));
	if (result == NULL)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	for (int i = 0; i < response->n_entries; i++)
	{
		void *value_data_dup = malloc(entries[i]->value.len);
		char *key_copy = malloc(strlen(entries[i]->key) + 1);
		if (value_data_dup == NULL || key_copy == NULL)
		{
			if (value_data_dup != NULL)
				free(value_data_dup);
			for (int j = 0; j < i; j++)
			{
				entry_destroy(result[j]);
			}
			free(result);
			message_t__free_unpacked(response, NULL);
			return NULL;
		}
		memcpy(value_data_dup, entries[i]->value.data, entries[i]->value.len);
		strcpy(key_copy, entries[i]->key);
		struct data_t *data = data_create(entries[i]->value.len, value_data_dup);
		result[i] = entry_create(key_copy, data);
		if (result[i] == NULL)
		{
			for (int j = 0; j < i; j++)
			{
				entry_destroy(result[j]);
			}
			free(result);
			message_t__free_unpacked(response, NULL);
			return NULL;
		}
	}
	message_t__free_unpacked(response, NULL);
	return result;
}

void rtable_free_entries(struct entry_t **entries)
{
	for (int i = 0; entries[i] != NULL; i++)
	{
		entry_destroy(entries[i]);
	}
	free(entries);
}

struct statistics_t *rtable_stats(struct rtable_t *rtable)
{
	if (rtable == NULL)
		return NULL;
	MessageT request;
	message_t__init(&request);
	request.opcode = MESSAGE_T__OPCODE__OP_STATS;
	request.c_type = MESSAGE_T__C_TYPE__CT_NONE;
	MessageT *response = network_send_receive(rtable, &request);
	if (response == NULL)
		return NULL;
	if (response->opcode != request.opcode + 1 || response->c_type != MESSAGE_T__C_TYPE__CT_STATS)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	struct statistics_t *statistics = (struct statistics_t *)malloc(sizeof(struct statistics_t));
	if (statistics == NULL)
	{
		message_t__free_unpacked(response, NULL);
		return NULL;
	}
	statistics->num_operations = response->statistics->num_operations;
	statistics->clients_connected = response->statistics->clients_connected;
	statistics->execution_time = response->statistics->execution_time;
	message_t__free_unpacked(response, NULL);
	return statistics;
}