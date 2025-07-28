#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>

#define MAX_PORT 1024
#define THREAD_COUNT 50
#define TIMEOUT_SEC 1

char ipTarget[100];
int currentPort = 1;
pthread_mutex_t port_mutex = PTHREAD_MUTEX_INITIALIZER;

int try_connect_with_timeout(char *ip, int port) {
    int sockfd;
    struct sockaddr_in addr;
    struct timeval timeout;
    fd_set fdset;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;

    fcntl(sockfd, F_SETFL, O_NONBLOCK); 

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)); 

    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);

    timeout.tv_sec = TIMEOUT_SEC;
    timeout.tv_usec = 0;

    if (select(sockfd + 1, NULL, &fdset, NULL, &timeout) > 0) {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        close(sockfd);
        return (so_error == 0);
    } else {
        close(sockfd);
        return 0; 
    }
}

void *thread_worker(void *arg) {
    int port;
    while (1) {
        pthread_mutex_lock(&port_mutex);
        if (currentPort > MAX_PORT) {
            pthread_mutex_unlock(&port_mutex);
            break;
        }
        port = currentPort++;
        pthread_mutex_unlock(&port_mutex);

        if (try_connect_with_timeout(ipTarget, port)) {
            printf("%d/tcp open\n", port);
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_COUNT];

    printf("Type the IP address to scan: ");
    scanf("%s", ipTarget);

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, thread_worker, NULL);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Scan complete.\n");
    return 0;
}
