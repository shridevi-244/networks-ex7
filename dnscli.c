#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 8080

int main(int argc, char *argv[])
{
    int sockfd;

    struct sockaddr_in server;
    socklen_t serverSize;

    char domain[100];
    char response[100];

    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 0;
    }

    printf("\n================ CONCURRENT DNS CLIENT ================\n\n");

    printf("Enter Domain Name : ");
    scanf(" %[^\n]", domain);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(argv[1]);

    serverSize = sizeof(server);

    if (connect(sockfd,
                (struct sockaddr *)&server,
                serverSize) < 0)
    {
        printf("Connection failed!\n");
        close(sockfd);
        return 0;
    }

    send(sockfd,
         domain,
         strlen(domain) + 1,
         0);

    printf("\nDNS Request Sent...\n");

    memset(response, 0, sizeof(response));

    recv(sockfd,
         response,
         sizeof(response),
         0);

    printf("\n========================================\n");

    printf("Domain Name : %s\n", domain);
    printf("%s\n", response);

    printf("========================================\n");

    close(sockfd);

    return 0;
}
