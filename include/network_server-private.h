/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include "table.h"
#include "stats.h"
#include "synchronization.h"

// Estrutura que contem os parâmetros de uma thread.
struct thread_info
{
    int client_socket;
    struct table_t *table;
    struct statistics_t *stats;
    struct table_synchronize_t *sync;
};

#endif