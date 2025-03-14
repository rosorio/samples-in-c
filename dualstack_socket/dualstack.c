#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    int dualstack = 0; // Allow both IPv4 and IPv6 (default on Linux)
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &dualstack, sizeof(dualstack));

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;
    addr.sin6_port = htons(8080);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(sockfd, 5);
    printf("Server listening on both IPv4 and IPv6\n");

    while (1) {
        struct sockaddr_in6 client;
        socklen_t len = sizeof(client);
        int client_fd = accept(sockfd, (struct sockaddr*)&client, &len);
        if (client_fd >= 0) {
            char ip[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &client.sin6_addr, ip, sizeof(ip));
            printf("Client connected: %s\n", ip);
            close(client_fd);
        }
    }

    close(sockfd);
    return 0;
}
