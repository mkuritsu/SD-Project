/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#ifndef STATS_H
#define STATS_H

#include <stdint.h>

// Estrutura que contém os campos necessários para a operação de stats.
struct statistics_t {
    int32_t num_operations;         // Número total de operações na tabela executadas no servidor.
    int32_t clients_connected;      // Tempo total acumulado gasto na execução de operações na tabela.
    int64_t execution_time;         // Número de clientes atualmente ligados ao servidor.
};

#endif
