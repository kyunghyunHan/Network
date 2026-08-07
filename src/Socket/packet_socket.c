#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 65536

int main(void)
{
    int sockfd;

    unsigned char buffer[BUFFER_SIZE];

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    printf("Packet Socket Started...\n");

    while (1)
    {
        ssize_t len = recv(sockfd, buffer, sizeof(buffer), 0);

        if (len < 0)
        {
            perror("recv");
            break;
        }

        printf("Packet Size : %zd bytes\n", len);
    }

    close(sockfd);

    return 0;
}