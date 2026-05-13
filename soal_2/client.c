#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_port = htons(9000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    printf("Connected to server\n");

    char buffer[4096];

    while (1)
    {
        printf("db > ");

        fgets(buffer, sizeof(buffer), stdin);

        send(sock, buffer, strlen(buffer), 0);

        int valread = read(sock, buffer, sizeof(buffer)-1);

        buffer[valread] = '\0';

        printf("%s\n", buffer);
    }

    close(sock);

    return 0;
}
