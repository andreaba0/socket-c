#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

int isLastWord(char **dest, char *string)
{
    free(*dest);
    if (string == NULL)
    {
        *dest = NULL;
        return -1;
    };
    if (string[strlen(string) - 1] == '\n')
    {
        *dest = malloc(strlen(string) - 1);
        strncpy(*dest, string, strlen(string) - 1);
        return 1;
    }
    else
    {
        *dest = strdup(string);
        return 0;
    }
}

typedef struct server_params {
    int maxTentativi;
    char*host;
    int port;
} server_args;

typedef struct pthread_arg_admin
{

} pthread_arg_admin;

typedef struct pthread_arg_t
{
    unsigned int client_sock;
    int maxTentativi;
} pthread_args;
char *parole[6] = {"etere", "fessa", "situa", "razzi", "mezzo", "tende"};
int thread_number = 0;
pthread_mutex_t thread_n;
int clientNumber = 0;

void *pthread_routine(void *args)
{
    pthread_args *localArgs = (pthread_args *)args;
    int maxTentativi = localArgs->maxTentativi;
    int client_sock = localArgs->client_sock;
    free(args);

    char server_message[256], client_message[256];
    memset(server_message, '\0', sizeof(server_message));
    pthread_mutex_lock(&thread_n);
    thread_number = thread_number + 1;
    sprintf(server_message, "OK %d Thread: %d, Inizio del gioco\n", maxTentativi, thread_number);
    pthread_mutex_unlock(&thread_n);
    if (write(client_sock, server_message, strlen(server_message)) < 0)
    {
        printf("Errore nell'invio del messaggio\n");
    }
    int parola = rand() % 6;

    for (int i = 0; i < maxTentativi; i++)
    {
        memset(client_message, '\0', sizeof(client_message));
        memset(server_message, '\0', sizeof(server_message));
        read(client_sock, client_message, sizeof(client_message));
        char *tempString = strdup(client_message);
        char *token = NULL;
        int status = isLastWord(&token, strtok(tempString, " "));
        if (status == -1)
        {
            sprintf(server_message, "ERR richiesto un comando\n");
            write(client_sock, server_message, strlen(server_message));
            break;
        }
        if (strcmp(token, "QUIT") == 0)
        {
            sprintf(server_message, "QUIT Alla prossima!\n");
            write(client_sock, server_message, strlen(server_message));
            break;
        }
        if (strcmp(token, "WORD") != 0)
        {
            sprintf(server_message, "ERR Comando '%s' sconosciuto\n", token);
            write(client_sock, server_message, strlen(server_message));
        }
        status = isLastWord(&token, strtok(NULL, " "));
        if (status != 1)
        {
            sprintf(server_message, "ERR Richiesta una parola\n");
            write(client_sock, server_message, strlen(server_message));
        }
        if (strlen(token) != 5)
        {
            sprintf(server_message, "ERR la parola deve essere di 5 caratteri\n");
            write(client_sock, server_message, strlen(server_message));
            break;
        }
        if (strcmp(token, parole[parola]) == 0)
        {
            sprintf(server_message, "OK PERFECT\n");
            write(client_sock, server_message, strlen(server_message));
            break;
        }

        if (i + 1 == maxTentativi)
        {
            memset(server_message, '\0', sizeof(server_message));
            sprintf(server_message, "END %d %s\n", maxTentativi, parole[parola]);
            write(client_sock, server_message, strlen(server_message));
            break;
        }

        char res[6];
        memset(res, '\0', sizeof(res));
        int err = 0;
        for (int j = 0; j < strlen(token); j++)
        {
            if (token[j] < 'a' || token[j] > 'z')
            {
                sprintf(server_message, "ERR caratteri accettati: 'a'->'z'\n");
                write(client_sock, server_message, strlen(server_message));
                err = 1;
                break;
            }
            if (token[j] == parole[parola][j])
            {
                res[j] = '*';
                continue;
            }
            int k;
            for (k = 0; k < strlen(parole[parola]); k++)
            {
                if (token[j] == parole[parola][k])
                {
                    res[j] = '+';
                    break;
                }
            }
            if (k != strlen(parole[parola]))
                continue;
            res[j] = '_';
        }
        if (err == 1)
            break;
        sprintf(server_message, "OK %d %s\n", i + 1, res);
        write(client_sock, server_message, strlen(server_message));
    }
    // Closing the socket:
    close(client_sock);
    return NULL;
}

void *pthread_server_routine(void *argv)
{
    server_args *args = (server_args*) argv;
    unsigned int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    pthread_t pthread;
    pthread_args *thread_args;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0)
    {
        printf("Error while creating socket\n");
        return NULL;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(args->port);
    server_addr.sin_addr.s_addr = inet_addr(args->host);

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Couldn't bind to the port\n");
        return NULL;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0)
    {
        printf("Error while listening\n");
        return NULL;
    }
    printf("\nListening for incoming connections.....\n");
    while (1)
    {

        // Accept an incoming connection:
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

        if (client_sock < 0)
        {
            printf("Can't accept\n");
            return NULL;
        }
        printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        thread_args = malloc(sizeof(pthread_args));
        thread_args->client_sock = client_sock;
        thread_args->maxTentativi = args->maxTentativi;
        pthread_create(&pthread, NULL, &pthread_routine, (void *)thread_args);
    }
    close(socket_desc);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int maxTentativi;
    pthread_t pthread_server;

    if (argc < 2 || argc > 4)
    {
        printf("./server.out <porta> <tentativi> <ip remoto>\n");
        return 1;
    }
    if (argc < 3)
        maxTentativi = 6;
    else if (atoi(argv[2]) < 6 || atoi(argv[2]) > 10)
    {
        printf("Numero di tentativi non valido\n");
        return 2;
    }
    else
        maxTentativi = atoi(argv[2]);

    char *host;
    if (argc != 4)
        host = strdup("127.0.0.1");
    else
        host = strdup(argv[3]);

    server_args*server_args = malloc(sizeof(server_args));

    server_args->maxTentativi=maxTentativi;
    server_args->host=host;
    server_args->port=atoi(argv[1]);

    int scelta;
    do {
        do {
            printf("\n\n");
            printf("Menu\n");
            printf("1) Avvia il server\n");
            printf("2) Chiudi il server\n");
            printf("3) Lista utenti arrivi\n");
            printf("4) Esci\n");
            scanf("%d", &scelta);
        }while(scelta<1||scelta>4);
        switch(scelta) {
            case 1:
                pthread_create(&pthread_server, NULL, &pthread_server_routine, (void*)server_args);
                break;
            case 2:
                pthread_cancel(pthread_server);
                break;
            case 3:
                printf("Empty\n");
        }
    }while(scelta!=4);
    //pthread_join(pthread_server, NULL);

    return 0;
}