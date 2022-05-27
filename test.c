#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int isLastWord(char **dest, char *string)
{
    if (string == NULL)
    {
        *dest = NULL;
        return -1;
    };
    if (string[strlen(string) - 1] == '\n') {
        *dest=malloc(strlen(string)-1);
        strncpy(*dest, string, strlen(string)-1);
        return 1;
    } else {
        *dest=strdup(string);
        return 0;
    }
}

int main(int argc, char *argv[])
{
    for(int i=0;i<10;i++) {
        if(i%2==0) continue;
        printf("n: %d\n", i);
    }
    printf("len: %ld\n", strlen("pro\nva\n"));
    char temp[10] = "testo di p";
    char *token = NULL;
    token = strtok("pro\nva\n", " ");
    printf("p: %s\n", token);
    isLastWord(&token, strtok(temp, " "));
    printf("%s\n", token);
    int status;
    while (token != NULL)
    {
        status = isLastWord(&token, strtok(NULL, " "));
        if (status == 1)
        {
            printf("%s\n", token);
            printf("End reached\n");
        }
        else if (status == -1)
            break;
        else
        {
            printf("%s\n", token);
        }
    }
    return 0;
}