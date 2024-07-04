/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#include <stdlib.h>
#include <memory.h>
#include "data.h"

struct data_t *data_create(int size, void *data)
{
	if (size <= 0 || data == NULL)
		return NULL;
	struct data_t *newData = (struct data_t*) malloc(sizeof(struct data_t));
	if (newData == NULL)
		return NULL;
	newData->datasize = size;
	newData->data = data;
	return newData;
}

int data_destroy(struct data_t *data)
{
	if (data == NULL || data->data == NULL)
		return -1;
	free(data->data);
	free(data);
	return 0;
}

struct data_t *data_dup(struct data_t *data)
{
	if (data == NULL || data->data == NULL || data->datasize <= 0 )
		return NULL;
	void *dup = (void*) malloc(data->datasize);
	if (dup == NULL)
		return NULL;
	memcpy(dup, data->data, data->datasize);
	return data_create(data->datasize, dup);
}

int data_replace(struct data_t *data, int new_size, void *new_data)
{
	if (data == NULL || data->data == NULL || new_size <= 0 || new_data == NULL)
		return -1;
	free(data->data);
	data->data = new_data;
	data->datasize = new_size;
	return 0;
}
