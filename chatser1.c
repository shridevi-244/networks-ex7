#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define PORT 9090
#define MAX 100
#define HISTORY_MAX 2048

void handle_client(
    int server_socket,
    struct sockaddr_in client,
    socklen_t client_len)
{
    char message[MAX];
    char history[HISTORY_MAX] = "";

    printf("\n[Child %d] Client connected!\n",
           getpid());

    while (1)
    {
        printf("[Child %d] Server : ",
               getpid());

        fgets(message, MAX, stdin);

        message[strcspn(message, "\n")] = '\0';

        strncat(history,
                "Server: ",
                HISTORY_MAX - strlen(history) - 1);

        strncat(history,
                message,
                HISTORY_MAX - strlen(history) - 1);

        strncat(history,
                "\n",
                HISTORY_MAX - strlen(history) - 1);

        sendto(server_socket,
               message,
               strlen(message) + 1,
               0,
               (struct sockaddr *)&client,
               client_len);

        if (strcmp(message, "exit") == 0)
        {
            printf("[Child %d] Chat ended.\n",
                   getpid());

            break;
        }

        memset(message, 0, sizeof(message));

        client_len = sizeof(client);

        recvfrom(server_socket,
                 message,
                 MAX,
                 0,
                 (struct sockaddr *)&client,
                 &client_len);

        if (strcmp(message, "exit") == 0)
        {
            printf("[Child %d] Client disconnected.\n",
                   getpid());

            break;
        }

        printf("Client : %s\n", message);

        strncat(history,
                "Client: ",
                HISTORY_MAX - strlen(history) - 1);

        strncat(history,
                message,
                HISTORY_MAX - strlen(history) - 1);

        strncat(history,
                "\n",
                HISTORY_MAX - strlen(history) - 1);

        printf("\n--- Chat History ---\n");
        printf("%s", history);
        printf("--------------------\n\n");
    }

    close(server_socket);

    exit(0);
}

int main()
{
    int server_socket;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t client_len;

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_socket < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket,
             (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        close(server_socket);
        return 1;
    }

    signal(SIGCHLD, SIG_IGN);

    printf("\n====================================\n");
    printf("CONCURRENT UDP CHAT SERVER\n");
    printf("Port : %d\n", PORT);
    printf("Waiting for clients...\n");
    printf("====================================\n");

    while (1)
    {
        char handshake[MAX];

        client_len = sizeof(client);

        memset(handshake, 0, sizeof(handshake));

        /*
         * Wait for client.
         */

        recvfrom(server_socket,
                 handshake,
                 MAX,
                 0,
                 (struct sockaddr *)&client,
                 &client_len);

        printf("\nNew client received.\n");

        /*
         * Create child.
         */

        pid_t pid = fork();

        if (pid < 0)
        {
            printf("Fork failed\n");
            continue;
        }

        if (pid == 0)
        {
            /*
             * Child handles client.
             */

            handle_client(server_socket,
                          client,
                          client_len);
        }

        /*
         * Parent continues accepting clients.
         */

        printf("[Parent] Client assigned to Child PID %d\n",
               pid);
    }

    close(server_socket);

    return 0;
}