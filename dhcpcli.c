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

    char clientName[100];
    int requestedSubnet;

    char ip[50];
    char mask[50];
    char gateway[50];

    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 0;
    }

    printf("\n================ CONCURRENT DHCP CLIENT ================\n\n");

    printf("Enter Client Name : ");
    scanf(" %[^\n]", clientName);

    printf("Enter Required Subnet Number : ");
    scanf("%d", &requestedSubnet);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

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

    sendto(sockfd,
           clientName,
           strlen(clientName) + 1,
           0,
           (struct sockaddr *)&server,
           serverSize);

    sendto(sockfd,
           &requestedSubnet,
           sizeof(requestedSubnet),
           0,
           (struct sockaddr *)&server,
           serverSize);

    printf("\nDHCP Request Sent...\n");

    memset(ip, 0, sizeof(ip));

    recvfrom(sockfd,
             ip,
             sizeof(ip),
             0,
             (struct sockaddr *)&server,
             &serverSize);

    if (strcmp(ip, "Invalid Subnet") == 0)
    {
        printf("\nInvalid Subnet Number!\n");
        close(sockfd);
        return 0;
    }

    memset(mask, 0, sizeof(mask));

    recvfrom(sockfd,
             mask,
             sizeof(mask),
             0,
             (struct sockaddr *)&server,
             &serverSize);

    memset(gateway, 0, sizeof(gateway));

    recvfrom(sockfd,
             gateway,
             sizeof(gateway),
             0,
             (struct sockaddr *)&server,
             &serverSize);

    printf("\n========================================\n");

    printf("Client Name       : %s\n", clientName);
    printf("Requested Subnet  : %d\n", requestedSubnet);

    printf("\nIP Address Assigned Successfully!\n");

    printf("Assigned IP       : %s\n", ip);
    printf("Subnet Mask       : %s\n", mask);
    printf("Gateway           : %s\n", gateway);

    close(sockfd);

    return 0;
}