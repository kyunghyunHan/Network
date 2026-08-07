#include <arpa/inet.h>   // sockaddr_in, inet_pton()
#include <stdio.h>       // printf(), perror(), fgets()
#include <string.h>      // memset(), strlen()
#include <sys/socket.h>  // socket(), connect(), recv(), send()
#include <unistd.h>      // close()

#define SERVER_IP "127.0.0.1"
// 라즈베리에서 실행하면 자기 자신
// 다른 PC에서 접속하면 라즈베리 IP로 변경
// 예) "192.168.219.110"

#define PORT 8080
#define BUFFER_SIZE 1024

int client(void)
{
    // 서버와 통신할 소켓
    int client_fd;

    // 서버 주소 정보
    struct sockaddr_in server_addr;

    // 송수신 버퍼
    char buffer[BUFFER_SIZE];

    /*
     * socket()
     *
     * AF_INET
     *      IPv4
     *
     * SOCK_STREAM
     *      TCP
     *
     * 성공하면 File Descriptor 반환
     */
    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    // 구조체 초기화
    memset(&server_addr, 0, sizeof(server_addr));

    // IPv4 사용
    server_addr.sin_family = AF_INET;

    // 서버 Port
    server_addr.sin_port = htons(PORT);

    /*
     * 문자열 IP를
     * 네트워크 주소(Binary)로 변환
     *
     * "127.0.0.1"
     *
     * ↓
     *
     * Binary IP
     */
    if (inet_pton(AF_INET,
                  SERVER_IP,
                  &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(client_fd);
        return 1;
    }

    /*
     * connect()
     *
     * 서버에게 연결 요청
     *
     * 내부적으로
     *
     * 3-Way Handshake 수행
     *
     * SYN
     * SYN + ACK
     * ACK
     */
    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(client_fd);
        return 1;
    }

    printf("Connected to server!\n");

    while (1)
    {
        printf("Input : ");

        // 키보드 입력
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;

        /*
         * send()
         *
         * 서버에게 데이터 전송
         */
        if (send(client_fd,
                 buffer,
                 strlen(buffer),
                 0) == -1)
        {
            perror("send");
            break;
        }

        /*
         * recv()
         *
         * 서버가 보내는 Echo 데이터 수신
         */
        ssize_t len = recv(client_fd,
                           buffer,
                           sizeof(buffer) - 1,
                           0);

        if (len == 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        if (len < 0)
        {
            perror("recv");
            break;
        }

        // 문자열 종료 문자 추가
        buffer[len] = '\0';

        printf("Server : %s", buffer);
    }

    // 서버 연결 종료
    close(client_fd);

    return 0;
}