#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 65536

int main(void)
{
    int raw_fd;

    unsigned char buffer[BUFFER_SIZE];

    /*
     * Raw Socket 생성
     *
     * AF_INET
     *   IPv4
     *
     * SOCK_RAW
     *   Raw Socket
     *
     * IPPROTO_ICMP
     *   ICMP 패킷만 받기
     */
    raw_fd = socket(
        AF_INET,
        SOCK_RAW,
        IPPROTO_ICMP
    );

    if (raw_fd == -1) {
        perror("socket");
        return 1;
    }

    printf("Raw ICMP socket started...\n");

    while (1)
    {
        struct sockaddr_in sender_addr;

        socklen_t sender_len = sizeof(sender_addr);

        /*
         * Raw Socket으로 패킷 수신
         *
         * buffer에는
         *
         * IP Header
         * +
         * ICMP Header
         * +
         * Data
         *
         * 가 들어온다.
         */
        ssize_t len = recvfrom(
            raw_fd,
            buffer,
            sizeof(buffer),
            0,
            (struct sockaddr *)&sender_addr,
            &sender_len
        );

        if (len < 0) {
            perror("recvfrom");
            break;
        }

        /*
         * buffer 시작 부분을
         * IP Header 구조체로 해석
         */
        struct iphdr *ip_header =
            (struct iphdr *)buffer;


        /*
         * IP Header 길이
         *
         * ihl 값은 32bit 단위이므로
         * 실제 byte 길이는 * 4
         */
        int ip_header_len =
            ip_header->ihl * 4;


        /*
         * IP Header 바로 다음 위치가
         * ICMP Header
         */
        struct icmphdr *icmp_header =
            (struct icmphdr *)
            (buffer + ip_header_len);


        printf("\nPacket received\n");

        printf(
            "From: %s\n",
            inet_ntoa(sender_addr.sin_addr)
        );

        printf(
            "IP Header Length: %d bytes\n",
            ip_header_len
        );

        printf(
            "ICMP Type: %d\n",
            icmp_header->type
        );

        printf(
            "ICMP Code: %d\n",
            icmp_header->code
        );

        printf(
            "Packet Length: %zd bytes\n",
            len
        );
    }

    close(raw_fd);

    return 0;
}