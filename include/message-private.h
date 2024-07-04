/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188 
* André Reis        58192
-------------------------------------------------------*/

#ifndef _MESSAGE_PRIVATE_H
#define  _MESSAGE_PRIVATE_H /* Módulo message */

/*
 * Recebe um buffer de qualquer dimensão pela rede.
 * Assegura que se retorna quando o buffer for totalmente recebido.
 * Retorna 0 ou -1 em caso de erro.
 */
int read_all(int sockfd, void *buffer, int size);

/*
 * Envia um buffer de qualquer dimensão pela rede.
 * Assegura que se retorna quando o buffer for totalmente enviado.
 * Retorna 0 ou -1 em caso de erro.
 */
int write_all(int sockfd, void *buffer, int size);

#endif

