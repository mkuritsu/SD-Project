/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#include <pthread.h>
#include <stdlib.h>
#include "synchronization.h"

struct table_synchronize_t *synchronize_create()
{
    struct table_synchronize_t *sync = (struct table_synchronize_t *) calloc(1, sizeof(struct table_synchronize_t));
    if (sync == NULL)
        return NULL;
    pthread_mutex_init(&sync->mutex, NULL);
    pthread_cond_init(&sync->cond, NULL);
    return sync;
}

void synchronize_destroy(struct table_synchronize_t *sync)
{
    if (sync == NULL)
        return;
    pthread_mutex_destroy(&sync->mutex);
    pthread_cond_destroy(&sync->cond);
    free(sync);
}

void begin_table_write(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    while(sync->state.is_writing_table || sync->state.num_reading_table > 0)
    {
        pthread_cond_wait(&sync->cond, &sync->mutex);
    }
    sync->state.is_writing_table = true;
    pthread_mutex_unlock(&sync->mutex);
}

void end_table_write(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    sync->state.is_writing_table = false;
    pthread_cond_broadcast(&sync->cond);
    pthread_mutex_unlock(&sync->mutex);
}

void begin_table_read(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    while(sync->state.is_writing_table)
    {
        pthread_cond_wait(&sync->cond, &sync->mutex);
    }
    sync->state.num_reading_table++;
    pthread_mutex_unlock(&sync->mutex);
}

void end_table_read(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    sync->state.num_reading_table--;
    if(sync->state.num_reading_table == 0)
        pthread_cond_broadcast(&sync->cond);
    pthread_mutex_unlock(&sync->mutex);
}

void begin_stats_write(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    while(sync->state.is_writting_stats || sync->state.num_reading_stats > 0)
    {
        pthread_cond_wait(&sync->cond, &sync->mutex);
    }
    sync->state.is_writting_stats = true;
    pthread_mutex_unlock(&sync->mutex);
}

void end_stats_write(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    sync->state.is_writting_stats = false;
    pthread_cond_broadcast(&sync->cond);
    pthread_mutex_unlock(&sync->mutex);
}

void begin_stats_read(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    while (sync->state.is_writting_stats)
    {
        pthread_cond_wait(&sync->cond, &sync->mutex);
    }
    sync->state.num_reading_stats++;
    pthread_mutex_unlock(&sync->mutex);
}

void end_stats_read(struct table_synchronize_t *sync)
{
    pthread_mutex_lock(&sync->mutex);
    sync->state.num_reading_stats--;
    if(sync->state.num_reading_stats == 0)
        pthread_cond_broadcast(&sync->cond);
    pthread_mutex_unlock(&sync->mutex);
}