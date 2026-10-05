#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>

#define PORT 8080
#define MAX 20

struct Subnet
{
    int network;
    int hostBits;
    int prefix;
    int total;
    int available;
    int block;
    int required;
};

int main()
{
    int sockfd;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t clientSize;

    struct Subnet subnet[MAX];

    int a, b, c, d;
    int prefix;
    int n;
    int i;

    printf("\n================ CONCURRENT DHCP SERVER ================\n\n");

    printf("Enter Base Address : ");
    scanf("%d.%d.%d.%d", &a, &b, &c, &d);

    printf("Enter Prefix : ");
    scanf("%d", &prefix);

    printf("Enter Number of Subnets : ");
    scanf("%d", &n);

    for (i = 0; i < n; i++)
    {
        printf("\nEnter Address Space Required for Subnet %d : ",
               i + 1);

        scanf("%d", &subnet[i].required);

        subnet[i].hostBits = 0;

        while ((1 << subnet[i].hostBits) - 2
               < subnet[i].required)
        {
            subnet[i].hostBits++;
        }

        subnet[i].prefix =
            32 - subnet[i].hostBits;

        subnet[i].total =
            1 << subnet[i].hostBits;

        subnet[i].available =
            subnet[i].total - 2;

        subnet[i].block =
            subnet[i].total;
    }

    printf("\n\nCalculating Subnet Details...\n\n");

    printf("Base Network : %d.%d.%d.%d/%d\n",
           a, b, c, d, prefix);

    printf("Number of Subnets : %d\n", n);

    int current = d;

    for (i = 0; i < n; i++)
    {
        printf("\n---------------------------------------------\n");

        printf("Subnet Number : %d\n", i + 1);

        printf("Address Space : %d hosts\n",
               subnet[i].required);

        printf("Host Bits : %d\n",
               subnet[i].hostBits);

        printf("Network Bits : %d\n",
               subnet[i].prefix);

        printf("New Prefix : /%d\n",
               subnet[i].prefix);

        printf("Total Addresses : %d\n",
               subnet[i].total);

        printf("Available Hosts : %d\n",
               subnet[i].available);

        if (subnet[i].prefix == 26)
            printf("Subnet Mask : 255.255.255.192\n");
        else if (subnet[i].prefix == 27)
            printf("Subnet Mask : 255.255.255.224\n");
        else if (subnet[i].prefix == 28)
            printf("Subnet Mask : 255.255.255.240\n");
        else if (subnet[i].prefix == 29)
            printf("Subnet Mask : 255.255.255.248\n");
        else
            printf("Subnet Mask : 255.255.255.252\n");

        printf("Network Address : %d.%d.%d.%d/%d\n",
               a,
               b,
               c,
               current,
               subnet[i].prefix);

        subnet[i].network = current;

        current += subnet[i].block;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd,
             (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        printf("Bind failed!\n");
        close(sockfd);
        return 0;
    }

    signal(SIGCHLD, SIG_IGN);

    printf("\n=============================================\n");
    printf("CONCURRENT DHCP SERVER READY\n");
    printf("Waiting for multiple clients...\n");

    while (1)
    {
        char clientName[100];
        int requestedSubnet;

        clientSize = sizeof(client);

        /*
         * Receive client name
         */

        memset(clientName, 0, sizeof(clientName));

        recvfrom(sockfd,
                 clientName,
                 sizeof(clientName),
                 0,
                 (struct sockaddr *)&client,
                 &clientSize);

        /*
         * Receive subnet number
         */

        recvfrom(sockfd,
                 &requestedSubnet,
                 sizeof(requestedSubnet),
                 0,
                 (struct sockaddr *)&client,
                 &clientSize);

        /*
         * Create child
         */

        pid_t pid = fork();

        if (pid < 0)
        {
            printf("Fork failed!\n");
            continue;
        }

        if (pid == 0)
        {
            char ip[50];
            char mask[50];
            char gateway[50];

            printf("\n[Child %d] Processing Client\n",
                   getpid());

            printf("Client Name : %s\n",
                   clientName);

            printf("Requested Subnet : %d\n",
                   requestedSubnet);

            if (requestedSubnet < 1 ||
                requestedSubnet > n)
            {
                strcpy(ip, "Invalid Subnet");

                sendto(sockfd,
                       ip,
                       strlen(ip) + 1,
                       0,
                       (struct sockaddr *)&client,
                       clientSize);

                printf("[Child %d] Invalid Subnet\n",
                       getpid());

                close(sockfd);
                exit(0);
            }

            i = requestedSubnet - 1;

            int assigned =
                subnet[i].network + 1;

            sprintf(ip,
                    "%d.%d.%d.%d",
                    a,
                    b,
                    c,
                    assigned);

            if (subnet[i].prefix == 26)
                strcpy(mask, "255.255.255.192");
            else if (subnet[i].prefix == 27)
                strcpy(mask, "255.255.255.224");
            else if (subnet[i].prefix == 28)
                strcpy(mask, "255.255.255.240");
            else if (subnet[i].prefix == 29)
                strcpy(mask, "255.255.255.248");
            else
                strcpy(mask, "255.255.255.252");

            strcpy(gateway, ip);

            printf("\n[Child %d] IP Address Assigned\n",
                   getpid());

            printf("IP       : %s\n", ip);
            printf("Mask     : %s\n", mask);
            printf("Gateway  : %s\n", gateway);

            sendto(sockfd,
                   ip,
                   strlen(ip) + 1,
                   0,
                   (struct sockaddr *)&client,
                   clientSize);

            sendto(sockfd,
                   mask,
                   strlen(mask) + 1,
                   0,
                   (struct sockaddr *)&client,
                   clientSize);

            sendto(sockfd,
                   gateway,
                   strlen(gateway) + 1,
                   0,
                   (struct sockaddr *)&client,
                   clientSize);

            close(sockfd);

            exit(0);
        }

        printf("[Parent] Client assigned to Child PID %d\n",
               pid);
    }

    close(sockfd);

    return 0;
}
