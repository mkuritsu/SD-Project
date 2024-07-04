/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#ifndef _ZOO_COMMON_H
#define _ZOO_COMMON_H

#include <zookeeper/zookeeper.h>
#include <stdint.h>
#include "client_stub.h"

// Constantes para o nome e a length dos nodes
#define RTABLE_ZOO_ROOT "/chain"
#define RTABLE_NODE_PATH "/chain/node"
#define ZOO_PATH_LEN 1024

/**
* Função que esstabelece a conexão entre o Zookeeper e a tabela.
* Retorna 0, ou -1 em caso de erro.
*/
int32_t connect_to_rtable(zhandle_t *zh, char *node_path, struct rtable_t **table);

#endif