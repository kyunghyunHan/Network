#include <arpa/inet.h>   // sockaddr_in, htons(), inet_pton()
#include <stdio.h>       // printf(), perror(), fgets()
#include <string.h>      // memset(), strlen()
#include <sys/socket.h>  // socket(), sendto(), recvfrom()
#include <unistd.h>      // close()

#define SERVER_IP "127.0.0.1"

// 맥북에서 실행해서 라즈베리 서버에 접속한다면
// 라즈베리 IP로 변경
//
// 예:
// #define SERVER_IP "192.168.219.110"

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    // UDP Client Socket
    int client_fd;

    // 서버 주소
    struct sockaddr_in server_addr;

    socklen_t server_len = sizeof(server_addr);

    // 송수신 버퍼
    char buffer[BUFFER_SIZE];


    /*
     * 1. UDP Socket 생성
     *
     * SOCK_DGRAM = UDP
     */
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (client_fd == -1) {
        perror("socket");
        return 1;
    }


    /*
     * 2. 서버 주소 초기화
     */
    memset(&server_addr, 0, sizeof(server_addr));

    // IPv4
    server_addr.sin_family = AF_INET;

    // Server Port 8080
    server_addr.sin_port = htons(PORT);


    /*
     * 문자열 IP
     *
     * "192.168.219.110"
     *
     * 를 실제 네트워크 주소 형식으로 변환
     */
    if (inet_pton(
            AF_INET,
            SERVER_IP,
            &server_addr.sin_addr
        ) <= 0) {

        perror("inet_pton");
        close(client_fd);
        return 1;
    }


    /*
     * UDP에는 TCP처럼 connect()가 반드시 필요하지 않다.
     *
     * 목적지 주소를 sendto() 할 때마다 넣어준다.
     */
    while (1)
    {
        printf("Input : ");

        // 키보드 입력
        if (fgets(
                buffer,
                sizeof(buffer),
                stdin
            ) == NULL) {

            break;
        }


        /*
         * 3. sendto()
         *
         * 특정 IP + Port로
         * UDP Datagram 전송
         */
        ssize_t sent = sendto(
            client_fd,
            buffer,
            strlen(buffer),
            0,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        );

        if (sent < 0) {
            perror("sendto");
            break;
        }


        /*
         * 4. recvfrom()
         *
         * UDP Server가 Echo한 데이터 받기
         */
        ssize_t len = recvfrom(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&server_addr,
            &server_len
        );

        if (len < 0) {
            perror("recvfrom");
            break;
        }


        // 문자열 끝 표시
        buffer[len] = '\0';

        printf("Server : %s", buffer);
    }


    close(client_fd);

    return 0;
}