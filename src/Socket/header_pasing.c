#include <arpa/inet.h>    // inet_ntoa()
#include <netinet/ip.h>   // struct iphdr
#include <netinet/tcp.h>  // struct tcphdr
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>   // socket(), recvfrom()
#include <unistd.h>       // close()

#define BUFFER_SIZE 65536

int main(void)
{
    int raw_fd;

    // 네트워크 패킷 원본 바이트를 저장할 버퍼
    unsigned char buffer[BUFFER_SIZE];

    /*
     * Raw Socket 생성
     *
     * AF_INET
     *      IPv4
     *
     * SOCK_RAW
     *      Raw Socket
     *
     * IPPROTO_TCP
     *      TCP 패킷만 수신
     */
    raw_fd = socket(
        AF_INET,
        SOCK_RAW,
        IPPROTO_TCP
    );

    if (raw_fd == -1) {
        perror("socket");
        return 1;
    }

    printf("TCP packet sniffer started...\n");

    while (1)
    {
        struct sockaddr_in sender_addr;
        socklen_t sender_len = sizeof(sender_addr);

        /*
         * 패킷 수신
         *
         * buffer 안에는:
         *
         * [ IP Header ][ TCP Header ][ Data ]
         *
         * 형태로 들어온다.
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


        // --------------------------------------------------
        // 1. IP Header 해석
        // --------------------------------------------------

        /*
         * buffer 시작 위치를
         * struct iphdr 포인터로 해석한다.
         */
        struct iphdr *ip_header =
            (struct iphdr *)buffer;


        /*
         * ihl
         *
         * IP Header Length
         *
         * 32bit word 단위이므로
         * 실제 byte 길이는 * 4
         */
        int ip_header_len =
            ip_header->ihl * 4;


        // --------------------------------------------------
        // 2. TCP Header 위치 계산
        // --------------------------------------------------

        /*
         * buffer 시작
         *
         * ↓
         *
         * IP Header
         *
         * ↓ ip_header_len 만큼 이동
         *
         * TCP Header
         */
        struct tcphdr *tcp_header =
            (struct tcphdr *)(buffer + ip_header_len);


        /*
         * TCP Header 길이
         *
         * doff도 32bit word 단위
         */
        int tcp_header_len =
            tcp_header->doff * 4;


        // --------------------------------------------------
        // 3. IP 주소 출력
        // --------------------------------------------------

        struct in_addr source_ip;
        struct in_addr destination_ip;

        source_ip.s_addr = ip_header->saddr;
        destination_ip.s_addr = ip_header->daddr;


        printf("\n=============================\n");

        printf(
            "Source IP      : %s\n",
            inet_ntoa(source_ip)
        );

        /*
         * inet_ntoa()는 내부 static buffer를 사용하므로
         * 연속 두 번 한 printf 안에서 호출하지 않는 게 안전하다.
         */
        printf(
            "Destination IP : %s\n",
            inet_ntoa(destination_ip)
        );


        // --------------------------------------------------
        // 4. IP Header 정보
        // --------------------------------------------------

        printf(
            "IP Version     : %d\n",
            ip_header->version
        );

        printf(
            "IP Header Len  : %d bytes\n",
            ip_header_len
        );

        printf(
            "Protocol       : %d\n",
            ip_header->protocol
        );

        printf(
            "TTL            : %d\n",
            ip_header->ttl
        );


        // --------------------------------------------------
        // 5. TCP Header 정보
        // --------------------------------------------------

        /*
         * 네트워크 바이트 순서이므로
         * ntohs() 필요
         */
        printf(
            "Source Port    : %u\n",
            ntohs(tcp_header->source)
        );

        printf(
            "Dest Port      : %u\n",
            ntohs(tcp_header->dest)
        );


        /*
         * Sequence Number와 ACK Number는 32bit
         *
         * ntohl()
         */
        printf(
            "Sequence       : %u\n",
            ntohl(tcp_header->seq)
        );

        printf(
            "ACK Number     : %u\n",
            ntohl(tcp_header->ack_seq)
        );


        printf(
            "TCP Header Len : %d bytes\n",
            tcp_header_len
        );


        // --------------------------------------------------
        // 6. TCP Flag 확인
        // --------------------------------------------------

        printf(
            "Flags          :"
        );

        if (tcp_header->syn)
            printf(" SYN");

        if (tcp_header->ack)
            printf(" ACK");

        if (tcp_header->fin)
            printf(" FIN");

        if (tcp_header->rst)
            printf(" RST");

        if (tcp_header->psh)
            printf(" PSH");

        if (tcp_header->urg)
            printf(" URG");

        printf("\n");


        // --------------------------------------------------
        // 7. Payload 크기 계산
        // --------------------------------------------------

        int payload_len =
            len
            - ip_header_len
            - tcp_header_len;


        printf(
            "Packet Length  : %zd bytes\n",
            len
        );

        printf(
            "Payload Length : %d bytes\n",
            payload_len
        );


        // --------------------------------------------------
        // 8. Payload 위치
        // --------------------------------------------------

        unsigned char *payload =
            buffer
            + ip_header_len
            + tcp_header_len;


        /*
         * Payload가 있으면 앞부분 출력
         *
         * HTTP 같은 평문 프로토콜이면
         * 내용을 볼 수 있다.
         */
        if (payload_len > 0)
        {
            printf("Payload        : ");

            int print_len = payload_len;

            // 너무 길면 앞 100바이트만 출력
            if (print_len > 100)
                print_len = 100;

            for (int i = 0; i < print_len; i++)
            {
                unsigned char c = payload[i];

                if (c >= 32 && c <= 126)
                    putchar(c);
                else
                    putchar('.');
            }

            printf("\n");
        }

        printf("=============================\n");
    }

    close(raw_fd);

    return 0;
}