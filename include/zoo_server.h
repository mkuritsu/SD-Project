/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#ifndef _ZOO_SERVER_H
#define _ZOO_SERVER_H

#include <zookeeper/zookeeper.h>
#include <stdint.h>
#include <table.h>
#include "client_stub.h"
#include "zoo_common.h"

/**
* Função que incializa um servidor com o Zookeeper e cria o nó correspondente.
* Retorna um pointer para a estrutura zhandle_t, ou NULL em caso de erro.
*/
zhandle_t *zoo_server_init(const char *host, const char *ip_addr, const int16_t port, struct table_t *table);

/**
* Getter para a variável que contém um pointer para a rtable do nó sucessor.
*/
struct rtable_t *zoo_server_get_successor();

void zoo_server_close();

#endif