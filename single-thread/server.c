#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

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

int main(int argc, char *argv[])
{
    int messageStatus;
    srand(time(NULL));
    char *parole[9] = {"etere", "stato", "cassa", "razzi", "mezzo", "tende", "sasso", "succo", "penna"};
    int socket_desc, client_sock;
    unsigned int client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[256], client_message[256];
    int maxTentativi;
    if (argc < 2 || argc > 3)
    {
        printf("./server.out <porta> <tentativi>\n");
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

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socket_desc < 0)
    {
        printf("Errore nella creazione della socket\n");
        return -1;
    }
    printf("Socket creata con successo\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Porta occupata\n");
        return -1;
    }

    // Listen for clients:
    if (listen(socket_desc, 1) < 0)
    {
        printf("Errore di ascolto\n");
        return -1;
    }
    printf("\nIn attesa di connessioni...\n");

    while (1)
    {

        // Accept an incoming connection:
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

        if (client_sock < 0)
        {
            printf("Can't accept\n");
            return -1;
        }
        printf("Client connesso con IP: %s e porta: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        memset(server_message, '\0', sizeof(server_message));
        sprintf(server_message, "OK %d Inizio del gioco\n", maxTentativi);
        if (write(client_sock, server_message, strlen(server_message)) < 0)
        {
            printf("Errore nell'invio del messaggio di apertura\n");
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
                messageStatus = write(client_sock, server_message, strlen(server_message));
                if (messageStatus < 0)
                {
                    printf("Errore nell'invio del messaggio\n");
                    i = maxTentativi + 1;
                    break;
                }
                break;
            }
            if (strcmp(token, "QUIT") == 0)
            {
                sprintf(server_message, "QUIT Alla prossima!\n");
                messageStatus = write(client_sock, server_message, strlen(server_message));
                if (messageStatus < 0)
                {
                    printf("Errore nell'invio del messaggio\n");
                    i = maxTentativi + 1;
                    break;
                }
                break;
            }
            if (strcmp(token, "WORD") != 0)
            {
                sprintf(server_message, "ERR Comando '%s' sconosciuto\n", token);
                messageStatus = write(client_sock, server_message, strlen(server_message));
                if (messageStatus < 0)
                {
                    printf("Errore nell'invio del messaggio\n");
                    i = maxTentativi + 1;
                    break;
                }
                break;
            }
            status = isLastWord(&token, strtok(NULL, " "));
            if (status != 1)
            {
                sprintf(server_message, "ERR Richiesta una parola\n");
                messageStatus = write(client_sock, server_message, strlen(server_message));
                if (messageStatus < 0)
                {
                    printf("Errore nell'invio del messaggio\n");
                    i = maxTentativi + 1;
                    break;
                }
                break;
            }
            if (strlen(token) != 5)
            {
                sprintf(server_message, "ERR la parola deve essere di 5 caratteri\n");
                messageStatus = write(client_sock, server_message, strlen(server_message));
                if (messageStatus < 0)
                {
                    printf("Errore nell'invio del messaggio\n");
                    i = maxTentativi + 1;
                    break;
                }
                break;
            }
            if (strcmp(token, parole[parola]) == 0)
            {
                sprintf(server_message, "OK PERFECT\n");
                messageStatus = write(client_sock, server_message, strlen(server_message));
                if (messageStatus < 0)
                {
                    printf("Errore nell'invio del messaggio\n");
                    i = maxTentativi + 1;
                    break;
                }
                break;
            }

            if (i + 1 == maxTentativi)
            {
                memset(server_message, '\0', sizeof(server_message));
                sprintf(server_message, "END %d %s\n", maxTentativi, parole[parola]);
                messageStatus = write(client_sock, server_message, strlen(server_message));
                if (messageStatus < 0)
                {
                    printf("Errore nell'invio del messaggio\n");
                    i = maxTentativi + 1;
                    break;
                }
                break;
            }

            char res[6];
            memset(res, '\0', sizeof(res));
            int err = 0;
            for (int j = 0; j < (int) strlen(token); j++)
            {
                if (token[j] < 'a' || token[j] > 'z')
                {
                    sprintf(server_message, "ERR caratteri accettati: 'a'->'z'\n");
                    messageStatus = write(client_sock, server_message, strlen(server_message));
                    if (messageStatus < 0)
                    {
                        printf("Errore nell'invio del messaggio\n");
                        i = maxTentativi + 1;
                        break;
                    }
                    err = 1;
                    break;
                }
                if (token[j] == parole[parola][j])
                {
                    res[j] = '*';
                    continue;
                }
                int k;
                for (k = 0; k < (int) strlen(parole[parola]); k++)
                {
                    if (token[j] == parole[parola][k])
                    {
                        res[j] = '+';
                        break;
                    }
                }
                if (k != (int) strlen(parole[parola]))
                    continue;
                res[j] = '_';
            }
            if (err == 1)
                break;
            sprintf(server_message, "OK %d %s\n", i + 1, res);
            messageStatus = write(client_sock, server_message, strlen(server_message));
            if (messageStatus < 0)
            {
                printf("Errore nell'invio del messaggio\n");
                i = maxTentativi + 1;
                break;
            }
        }

        // Closing the socket:
        close(client_sock);
    }
    close(socket_desc);

    return 0;
}