/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "sdmessage.pb-c.h"
#include "table.h"
#include "synchronization.h"

/* Executa na tabela table a operação de put e utiliza a mesma 
 * estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int handle_put(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync);

/* Executa na tabela table a operação de get e utiliza a mesma 
 * estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int handle_get(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync);

/* Executa na tabela table a operação de delete e utiliza a mesma 
 * estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int handle_del(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync);

/* Executa na tabela table a operação de size e utiliza a mesma 
 * estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int handle_size(MessageT *msg, struct table_t *table);

/* Executa na tabela table a operação de getkeys e utiliza a mesma 
 * estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int handle_getkeys(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync);

/* Executa na tabela table a operação de gettable e utiliza a mesma 
 * estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int handle_gettable(MessageT *msg, struct table_t *table, struct table_synchronize_t *sync);

#endif