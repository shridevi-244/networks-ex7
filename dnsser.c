#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080

struct DNS
{
    char domain[100];
    char ip[50];
};

struct DNS table[20] =
{
    {"google.com", "142.250.72.14"},
    {"youtube.com", "142.250.72.206"},
    {"facebook.com", "157.240.241.35"}
};

int count = 3;

void handleClient(int clientfd)
{
    char domain[100];
    char response[100];

    memset(domain, 0, sizeof(domain));

    recv(clientfd,
         domain,
         sizeof(domain),
         0);

    int found = 0;
    int i;
    for (i = 0; i < count; i++)
    {
        if (strcmp(table[i].domain, domain) == 0)
        {
            sprintf(response,
                    "IP Address: %s",
                    table[i].ip);

            found = 1;
            break;
        }
    }

    if (!found)
    {
        strcpy(table[count].domain, domain);

        sprintf(table[count].ip,
                "192.168.1.%d",
                count + 1);

        sprintf(response,
                "Domain Added. IP Address: %s",
                table[count].ip);

        count++;
    }

    send(clientfd,
         response,
         strlen(response) + 1,
         0);

    close(clientfd);
    exit(0);
}

int main()
{
    int sockfd, clientfd;

    struct sockaddr_in server, client;
    socklen_t clientSize;

    printf("\n================ CONCURRENT DNS SERVER ================\n\n");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(sockfd,
         (struct sockaddr *)&server,
         sizeof(server));

    listen(sockfd, 5);

    printf("DNS Server Waiting on Port %d...\n", PORT);

    while (1)
    {
        clientSize = sizeof(client);

        clientfd = accept(sockfd,
                          (struct sockaddr *)&client,
                          &clientSize);

        if (clientfd < 0)
        {
            printf("Accept failed!\n");
            continue;
        }

        printf("Client Connected...\n");

        /*
           Create a separate process
           for each client
        */
        if (fork() == 0)
        {
            close(sockfd);

            handleClient(clientfd);
        }

        close(clientfd);
    }

    close(sockfd);

    return 0;
}