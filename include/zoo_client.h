/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#ifndef _ZOO_CLIENT_H
#define _ZOO_CLIENT_H

#include <zookeeper/zookeeper.h>
#include "client_stub.h"
#include "zoo_common.h"


/**
* Função que inicializa um cliente com zookeeper e o conecta ao servidor.
* Retorna NULL em caso de erro ou um pointer para a struct zhandle_t.
*/
zhandle_t *zoo_client_init(const char *host);

/**
* Getter para a variavel que contém a tabela do servidor de escrita (head).
*/
struct rtable_t *zoo_client_get_write_table(void);

/**
* Getter para a variavel que contém a tabela do servidor de leitura (tail).
*/
struct rtable_t *zoo_client_get_read_table(void);

void zoo_client_close();

#endif