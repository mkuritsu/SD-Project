/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "zoo_common.h"

int32_t connect_to_rtable(zhandle_t *zh, char *node_path, struct rtable_t **table)
{
    int32_t addr_len = ZOO_PATH_LEN;
    char addr[addr_len];
    if (zoo_get(zh, node_path, 0, addr, &addr_len, NULL) != ZOK)
        return -1;
    if (*table != NULL)
        rtable_disconnect(*table);
    *table = rtable_connect(addr);
    if (*table == NULL)
        return -1;
    return 0;
}