/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "list.h"

struct table_t {
	struct list_t **lists;
	int size;
};

/* Função que calcula o índice da lista a partir da chave.
*/
int hash_code(char *key, int n);

#endif