/*-------------------------------------------------------
* Projeto desenvolvido por:
*
* Grupo 20:
* Rodrigo Correia   58180
* Laura Cunha       58188
* André Reis        58192
-------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include "client_stub.h"
#include "zoo_client.h"

static char *s_console_line = NULL;
static zhandle_t *s_zh = NULL;

void print_menu()
{
    puts(">----------[ HELP MENU ]----------<");
    puts(" - p[ut] <key> <value> - adds a pair (key, value) to the table.");
    puts(" - g[et] <key>         - gets a value from the table using the key.");
    puts(" - d[el] <key>         - deletes an entry from the table.");
    puts(" - s[ize]              - get the size of the table.");
    puts(" - [get]k[eys]         - get a list of all keys in the table.");
    puts(" - [get]t[able]        - get the complete table representation.");
    puts(" - st[ats]             - get all statistics from the server.");
    puts(" - h[elp]              - show the help menu.");
    puts(" - q[uit]              - quits the application.");
}

// Read until the end of data including spaces
char *read_data_argument()
{
    char *data = strtok(NULL, " ");
    if (data == NULL)
        return NULL;
    char *next;
    do
    {
        next = strtok(NULL, " ");
        if (next != NULL)
            *(next - 1) = ' ';
    } while (next != NULL);
    return data;
}

void handler_put(struct rtable_t *rtable)
{
    char *key = strtok(NULL, " ");
    char *value = read_data_argument();
    if (key == NULL || value == NULL)
    {
        puts("(!) Error: invalid arguments!");
        puts("(!) Usage: put <key> <value>");
        return;
    }
    int value_size = strlen(value);
    char *key_dup = (char *)calloc(strlen(key) + 1, sizeof(char));
    if (key_dup == NULL)
    {
        fprintf(stderr, "(!) Error: copying key from arguments!\n");
        return;
    }
    strcpy(key_dup, key);
    char *value_dup = (char *)calloc(value_size, sizeof(char));
    if (value_dup == NULL)
    {
        free(key_dup);
        fprintf(stderr, "(!) Error: copying value from arguments!\n");
        return;
    }
    memcpy(value_dup, value, value_size);
    struct data_t *data = data_create(value_size, value_dup);
    if (data == NULL)
    {
        free(key_dup);
        free(value_dup);
    }
    struct entry_t *entry = entry_create(key_dup, data);
    if (entry == NULL)
    {
        free(key_dup);
        data_destroy(data);
    }
    if (rtable_put(rtable, entry) == -1)
    {
        fprintf(stderr, "(!) Error: put is not available!\n");
        entry_destroy(entry);
        return;
    }
    puts(">----------[ PUT ]----------<");
    printf("    Entry (%s, %s) has been added to the table (or updated).\n", key, value);
    entry_destroy(entry);
}

void handler_get(struct rtable_t *rtable)
{
    char *key = strtok(NULL, " ");
    if (key == NULL)
    {
        fprintf(stderr, "(!) Error: invalid arguments!\n");
        puts("(!) Usage: get <key>");
        return;
    }
    struct data_t *data = rtable_get(rtable, key);
    if (data == NULL)
    {
        fprintf(stderr, "(!) Error: Couldn't get the entry from table! Key not found or error in rtable_get.\n");
        return;
    }
    uint8_t *data_content = (uint8_t *) data->data;
    puts(">----------[ GET ]----------<");
    puts("    | Value (ASCII) | Value (HEX) |");
    puts("    -------------------------------");
    printf("    | ");
    for (int i = 0; i < data->datasize; i++)
    {
        printf("%c", data_content[i]);
    }
    printf(" | ");
    for (int i = 0; i < data->datasize; i++)
    {
        printf("%X", data_content[i]);
        if (i != data->datasize - 1)
            printf(", ");
    }
    printf(" |\n");
    data_destroy(data);
}

void handler_del(struct rtable_t *rtable)
{
    char *key = strtok(NULL, " ");
    if (key == NULL)
    {
        fprintf(stderr, "(!) Error: invalid arguments!\n");
        puts("(!) Usage: del <key>");
        return;
    }
    int delete = rtable_del(rtable, key);
    if (delete == -1)
    {
        fprintf(stderr, "(!) Error: Couldn't delete the entry from table! Key not found or error in rtable_del.\n");
    }
    else
    {
        puts(">----------[ DELETE ]----------<");
        printf("    Entry %s deleted successfully\n", key);
    }
}

void handler_size(struct rtable_t *rtable)
{
    int size = rtable_size(rtable);
    if (size == -1)
    {
        fprintf(stderr, "(!) Error: table size is not available!\n");
        return;
    }
    else
    {
        puts(">----------[ SIZE ]----------<");
        printf("    The table size is %d\n", size);
    }
}

void handler_get_keys(struct rtable_t *rtable)
{
    char **keys = rtable_get_keys(rtable);
    if (keys == NULL)
    {
        fprintf(stderr, "(!) Error: get keys is not available!\n");
        return;
    }
    if (keys[0] == NULL)
    {
        puts("There isn't any key in the table!");
        rtable_free_keys(keys);
        return;
    }
    puts(">----------[ KEYS ]----------<");
    for (int i = 0; keys[i] != NULL; i++)
    {
        printf("    - %s\n", keys[i]);
    }
    rtable_free_keys(keys);
}

void handler_get_table(struct rtable_t *rtable)
{
    struct entry_t **table = rtable_get_table(rtable);
    if (table == NULL)
    {
        fprintf(stderr, "(!) Error: get table is not available!\n");
        return;
    }
    if (table[0] == NULL)
    {
        puts("Table is empty!");
        rtable_free_entries(table);
        return;
    }
    puts(">----------[ TABLE ]----------<");
    puts("    | Key | Value (ASCII) | Value (HEX) |");
    puts("    -------------------------------------");

    for (int i = 0; table[i] != NULL; i++)
    {
        struct data_t *data = table[i]->value;
        uint8_t *data_content = (uint8_t *) data->data;
        printf("    | %s | ", table[i]->key);
        for (int j = 0; j < data->datasize; j++)
        {
            printf("%c", data_content[j]);
        }
        printf(" | ");
        for (int j = 0; j < data->datasize; j++)
        {
            printf("%X", data_content[j]);
            if (j != data->datasize - 1)
                printf(", ");
        }
        printf(" |\n");
    }
    rtable_free_entries(table);
}

void handler_stats(struct rtable_t *rtable)
{
	struct statistics_t *stats = rtable_stats(rtable);
	if(stats == NULL)
	{
		fprintf(stderr, "(!) Error: get stats is not available!");
		return;
	}
	puts(">-----------[ STATS ]-----------<");
	printf("Number of operations: %d \n", stats->num_operations);
	printf("Number of clients connected: %d \n", stats->clients_connected);
	printf("Execution time: %ld us\n", stats->execution_time);
	free(stats);
}

void ctrlC(int sig)
{
    signal(SIGINT, ctrlC);
    puts("Quitting application.");
    free(s_console_line);
    zoo_client_close();
    zookeeper_close(s_zh);
    exit(EXIT_SUCCESS);
}

void print_usage(int argc, char **argv)
{
    char *binaryName;
    if (argc > 0)
        binaryName = argv[0];
    else
        binaryName = "./table-client";
    printf("Usage: %s <server_ip>:<port_server>\n", binaryName);
    printf("e.g: %s <127.0.0.1>:<1024>\n", binaryName);
}

void read_infinity_console_line()
{
    const int BUFFER_SIZE = 20;
    int currentSize = BUFFER_SIZE;
    s_console_line = (char *)calloc(BUFFER_SIZE, sizeof(char));
    if (s_console_line == NULL)
        return;
    int i = 0;
    char c;
    while ((c = fgetc(stdin)) != '\n')
    {
        s_console_line[i] = c;
        i++;
        if (i == currentSize - 1) // if there's no space left
        {
            s_console_line = (char *)realloc(s_console_line, currentSize + BUFFER_SIZE);
            if (s_console_line == NULL)
            {
                free(s_console_line);
                s_console_line = NULL;
                return;
            }
            currentSize += BUFFER_SIZE;
        }
    }
    if (c == EOF)
    {
        free(s_console_line);
        s_console_line = NULL;
        return;
    }
    // Resize by removing excess memory
    s_console_line = (char *)realloc(s_console_line, i + 1);
    if (s_console_line == NULL)
    {
        free(s_console_line);
        s_console_line = NULL;
        return;
    }
    s_console_line[i] = '\0';
}

void choose_option(char *option)
{
    if (option == NULL)
    {
        puts("(!) Invalid option!");
        return;
    }
    if (!strcmp(option, "put") || !strcmp(option, "p"))
        handler_put(zoo_client_get_write_table());
    else if (!strcmp(option, "get") || !strcmp(option, "g"))
        handler_get(zoo_client_get_read_table());
    else if (!strcmp(option, "del") || !strcmp(option, "d"))
        handler_del(zoo_client_get_write_table());
    else if (!strcmp(option, "size") || !strcmp(option, "s"))
        handler_size(zoo_client_get_read_table());
    else if (!strcmp(option, "getkeys") || !strcmp(option, "k"))
        handler_get_keys(zoo_client_get_read_table());
    else if (!strcmp(option, "gettable") || !strcmp(option, "t"))
        handler_get_table(zoo_client_get_read_table());
    else if (!strcmp(option, "stats") || !strcmp(option, "st"))
        handler_stats(zoo_client_get_read_table());
    else if (!strcmp(option, "help") || !strcmp(option, "h"))
        print_menu();
    else if (!strcmp(option, "quit") || !strcmp(option, "q"))
        ctrlC(SIGINT);
    else
        puts("(!) Invalid option!");
}

int main(int argc, char **argv)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, ctrlC);
    if (argc != 2)
    {
        print_usage(argc, argv);
        exit(EXIT_FAILURE);
    }
    s_zh = zoo_client_init(argv[1]);
    if (s_zh == NULL)
    {
        fprintf(stderr, "Failed to connect to ZooKeeper!\n");
        exit(EXIT_FAILURE);
    }
    print_menu();
    while (1)
    {
        printf("\n$> ");
        read_infinity_console_line();
        puts("");
        if (s_console_line == NULL)
        {
            fprintf(stderr, "(!) Failed to read standard input!\n");
            zookeeper_close(s_zh);
            exit(EXIT_FAILURE);
        }
        char *option = strtok(s_console_line, " ");
        choose_option(option);
        free(s_console_line);
        s_console_line = NULL;
    }
    zookeeper_close(s_zh);
    return 0;
}
