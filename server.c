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
    srand(time(NULL));
    char *parole[6] = {"etere", "fessa", "situa", "razzi", "mezzo", "tende"};
    unsigned int socket_desc, client_sock, client_size;
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
    else
        maxTentativi = atoi(argv[2]);

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0)
    {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0)
    {
        printf("Error while listening\n");
        return -1;
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
            return -1;
        }
        printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        memset(server_message, '\0', sizeof(server_message));
        sprintf(server_message, "OK %s Inizio del gioco\n", argv[2]);
        if (send(client_sock, server_message, strlen(server_message), 0) < 0)
        {
            printf("Errore nell'invio del messaggio\n");
        }
        int parola = rand() % 6;

        for (int i = 0; i < maxTentativi; i++)
        {
            memset(client_message, '\0', sizeof(client_message));
            memset(server_message, '\0', sizeof(server_message));
            recv(client_sock, client_message, sizeof(client_message), 0);
            char *tempString = strdup(client_message);
            char *token = NULL;
            int status = isLastWord(&token, strtok(tempString, " "));
            if (status == -1 || status == 1)
            {
                sprintf(server_message, "ERR comando: '%s' sconosciuto\n", token);
                send(client_sock, server_message, strlen(server_message), 0);
            }
            if (strcmp(token, "QUIT") == 0)
            {
                sprintf(server_message, "QUIT Alla prossima!\n");
                send(client_sock, server_message, strlen(server_message), 0);
                break;
            }
            if (strcmp(token, "WORD") != 0)
            {
                sprintf(server_message, "ERR Comando '%s' sconosciuto\n", token);
                send(client_sock, server_message, strlen(server_message), 0);
            }
            status = isLastWord(&token, strtok(NULL, " "));
            if (status != 1)
            {
                sprintf(server_message, "ERR Richiesta una parola\n");
                send(client_sock, server_message, strlen(server_message), 0);
            }
            if (strlen(token) != 5)
            {
                sprintf(server_message, "ERR la parola deve essere di 5 caratteri\n");
                send(client_sock, server_message, strlen(server_message), 0);
                break;
            }
            if (strcmp(token, parole[parola]) == 0)
            {
                sprintf(server_message, "OK PERFECT\n");
                send(client_sock, server_message, strlen(server_message), 0);
                break;
            }

            if (i + 1 == maxTentativi)
            {
                memset(server_message, '\0', sizeof(server_message));
                sprintf(server_message, "END %d %s\n", maxTentativi, parole[parola]);
                send(client_sock, server_message, strlen(server_message), 0);
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
                    send(client_sock, server_message, strlen(server_message), 0);
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
            send(client_sock, server_message, strlen(server_message), 0);
        }

        // Closing the socket:
        close(client_sock);
    }
    close(socket_desc);

    return 0;
}