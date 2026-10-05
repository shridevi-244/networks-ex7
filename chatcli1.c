#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define PORT 9090
#define MAX 100

int main()
{
    int client_socket;

    char message[MAX];

    struct sockaddr_in server;
    socklen_t server_len;

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    server_len = sizeof(server);

    printf("CONCURRENT UDP CHAT CLIENT STARTED...\n");

    /*
     * Child receives messages.
     */

    pid_t pid = fork();

    if (pid < 0)
    {
        printf("Fork failed\n");
        close(client_socket);
        return 1;
    }

    if (pid == 0)
    {
        while (1)
        {
            memset(message, 0, sizeof(message));

            recvfrom(client_socket,
                     message,
                     MAX,
                     0,
                     NULL,
                     NULL);

            if (strcmp(message, "exit") == 0)
            {
                printf("\n[Server] Chat ended.\n");
                exit(0);
            }

            printf("\nServer : %s\n", message);
            printf("Client : ");
            fflush(stdout);
        }
    }
    else
    {
        /*
         * Parent sends messages.
         */

        while (1)
        {
            printf("Client : ");

            fgets(message, MAX, stdin);

            message[strcspn(message, "\n")] = '\0';

            sendto(client_socket,
                   message,
                   strlen(message) + 1,
                   0,
                   (struct sockaddr *)&server,
                   server_len);

            if (strcmp(message, "exit") == 0)
            {
                kill(pid, SIGTERM);
                break;
            }
        }

        wait(NULL);
    }

    close(client_socket);

    return 0;
}