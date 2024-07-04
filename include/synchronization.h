/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#ifndef _SYNCHRONIZATION_H
#define _SYNCHRONIZATION_H

#include <pthread.h>
#include <stdbool.h>

// Estrutura que contém os estados dos acessos à tabela do servidor.
struct table_server_state
{
    int num_reading_table;      // Número de leituras
    bool is_writing_table;      // Verifica se alguma thread está a escrever
    int num_reading_stats;      // Número de leituras dos stats
    bool is_writting_stats;     // Verifica se alguma thread está a escrever stats
};

// Estrutura para sincronização.
struct table_synchronize_t
{
    struct table_server_state state;
    pthread_mutex_t mutex;              // Mutex
    pthread_cond_t cond;                // Conditional variable
};

/* Inicializa uma estrutura table_synchronize_t, 
* inicializando o respetivo mutex e a variável condicional.
*/
struct table_synchronize_t *synchronize_create();

/* Destrói uma estrutura table_synchronize_t, 
* destruindo o respetivo mutex e a variável condicional.
*/
void synchronize_destroy(struct table_synchronize_t *sync);

/* Gere a concorrência de escritas no acesso à tabela.
*  Apenas permite que uma thread escreva nesta com exclusão mútua.
*/
void begin_table_write(struct table_synchronize_t *sync);

/* Gere a concorrência de escritas no acesso à tabela.
*  Notifica as threads que a escrita terminou.
*/
void end_table_write(struct table_synchronize_t *sync);

/* Gere a concorrência de leituras no acesso à tabela.
*  Apenas permite que uma thread leia nesta com exclusão mútua.
*/
void begin_table_read(struct table_synchronize_t *sync);

/* Gere a concorrência de escritas no acesso à tabela.
*  Notifica as threads que a leitura terminou.
*/
void end_table_read(struct table_synchronize_t *sync);

/* Gere a concorrência de escritas na estrutura de estatísticas.
*  Apenas permite que uma thread aceda à mesma com exclusão mútua.
*/
void begin_stats_write(struct table_synchronize_t *sync);

/* Gere a concorrência de escritas no acesso à estrutura de estatísticas.
*  Notifica as threads que a escrita terminou.
*/
void end_stats_write(struct table_synchronize_t *sync);

/* Gere a concorrência de escritas na estrutura de estatísticas.
*  Apenas permite que uma thread aceda à mesma com exclusão mútua.
*/
void begin_stats_read(struct table_synchronize_t *sync);

/* Gere a concorrência de escritas no acesso à estrutura de estatísticas.
*  Notifica as threads que a escrita terminou.
*/
void end_stats_read(struct table_synchronize_t *sync);

#endif