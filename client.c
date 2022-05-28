#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

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
    if (argc != 3)
    {
        printf("Richiesto: ./programma <indirizzo_server> <numero_porta>\n");
        return 1;
    }

    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[256], client_message[256];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0)
    {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");
    int status;
    char *token = NULL;
    char tempString[6];
    memset(tempString, '\0', sizeof(tempString));

    read(socket_desc, server_message, sizeof(server_message));
    status = isLastWord(&token, strtok(strdup(server_message), " "));
    status = isLastWord(&token, strtok(NULL, " "));
    int maxTentativi = atoi(token);
    printf("Inizio del Gioco\n");
    printf("Numero di tentativi: %d\n", maxTentativi);
    printf("Messaggio dal server: ");
    status = isLastWord(&token, strtok(NULL, " "));
    while (status != -1)
    {
        if (status == 1)
            printf("%s\n", token);
        else if (status == 0)
            printf("%s ", token);
        status = isLastWord(&token, strtok(NULL, " "));
    }

    int scelta;
    do
    {
        memset(server_message, '\0', sizeof(server_message));
        memset(client_message, '\0', sizeof(client_message));
        do
        {
            printf("1: Inserisci parola\n");
            printf("2: Esci\n");
            printf("Enter message: ");
            scanf("%d", &scelta);
        } while (scelta < 1 || scelta > 2);
        switch (scelta)
        {
        case 1:
            printf("Parola: ");
            scanf("%s", tempString);
            sprintf(client_message, "WORD %s\n", tempString);
            write(socket_desc, client_message, strlen(client_message));
            read(socket_desc, server_message, sizeof(server_message));
            status = isLastWord(&token, strtok(strdup(server_message), " "));
            if (strcmp(token, "ERR") == 0)
            {
                printf("Errore: ");
                status = isLastWord(&token, strtok(NULL, " "));
                while (status != -1)
                {
                    if (status == 1)
                        printf("%s\n", token);
                    else if (status == 0)
                        printf("%s ", token);
                    status = isLastWord(&token, strtok(NULL, " "));
                }
                scelta = 2;
                break;
            }
            if (strcmp(token, "END") == 0)
            {
                printf("Numero tentativi raggiunto\n");
                status = isLastWord(&token, strtok(NULL, " "));
                status = isLastWord(&token, strtok(NULL, " "));
                printf("La parola era: %s\n", token);
                scelta = 2;
                break;
            }
            status = isLastWord(&token, strtok(NULL, " "));
            if (strcmp(token, "PERFECT") == 0)
            {
                printf("Parola indovinata\n");
                scelta = 2;
                break;
            }
            printf("Numero tentativo: %d/%d\n", atoi(token), maxTentativi);
            status = isLastWord(&token, strtok(NULL, " "));
            printf("Parola: %s\n", token);
            break;
        case 2:
            printf("Uscita in corso\n");
            sprintf(client_message, "QUIT\n");
            write(socket_desc, client_message, strlen(client_message));
            read(socket_desc, server_message, sizeof(server_message));
            status = isLastWord(&token, strtok(strdup(server_message), " "));
            status = isLastWord(&token, strtok(NULL, " "));
            while (status != -1)
            {
                if (status == 1)
                    printf("%s\n", token);
                else if (status == 0)
                    printf("%s ", token);
                status = isLastWord(&token, strtok(NULL, " "));
            }
            break;
        }
        printf("\n\n\n");
    } while (scelta != 2);

    // Close the socket:
    close(socket_desc);

    return 0;
}