#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
int main() {
    int port, openCount = 0, closeCount = 0;
    char ipTarget[100];
    int openPorts[1024], closedPorts[1024];

    printf("Type the IP address to scan: ");
    scanf("%s", ipTarget);

    for (port = 1; port <= 1024; port++) {
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            perror("socket");
            return 1;
        }

        struct sockaddr_in target;
        memset(&target, 0, sizeof(target));
        target.sin_family = AF_INET;
        target.sin_port = htons(port);

        if (inet_pton(AF_INET, ipTarget, &target.sin_addr) <= 0) {
            perror("inet_pton");
            close(socket_fd);
            continue;
        }

        int result = connect(socket_fd, (struct sockaddr*)&target, sizeof(target));
        if (result == 0) {
            openPorts[openCount++] = port;
            printf("%d/tcp open\n", port);
        } else {
            closedPorts[closeCount++] = port;
        }

        close(socket_fd);
    }

    return 0;
}
