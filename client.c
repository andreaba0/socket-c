#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

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

    recv(socket_desc, server_message, sizeof(server_message), 0);
    char *tempString = strdup(server_message);
    char *token = strtok(tempString, " ");
    token = strtok(NULL, " ");
    int maxTentativi = atoi(token);
    printf("Inizio del Gioco\n");
    printf("Numero di tentativi: %d\n", maxTentativi);
    token = strtok(NULL, " ");
    printf("Messaggio dal server: ");
    while (token != NULL)
    {
        printf("%s", token);
        token = strtok(NULL, " ");
        if (token != NULL)
            printf(" ");
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
            sprintf(client_message, "WORD %s", tempString);
            send(socket_desc, client_message, strlen(client_message), 0);
            recv(socket_desc, server_message, sizeof(server_message), 0);
            printf("Server: %s\n", server_message);
            //tempString = strdup(server_message);
            //token = strtok(tempString, " ");
            break;
        case 2:
            printf("Uscita in corso\n");
            sprintf(client_message, "QUIT");
            send(socket_desc, client_message, strlen(client_message), 0);
            recv(socket_desc, server_message, sizeof(server_message), 0);
            tempString = strdup(server_message);
            token = strtok(tempString, " ");
            token = strtok(NULL, " ");
            while (token != NULL)
            {
                printf("%s", token);
                token = strtok(NULL, " ");
                if (token != NULL)
                    printf(" ");
            }
            break;
        }
    } while (scelta != 2);

    // Close the socket:
    close(socket_desc);

    return 0;
}