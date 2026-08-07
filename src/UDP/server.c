#include <arpa/inet.h>   // sockaddr_in, htons(), htonl(), inet_ntoa()
#include <stdio.h>       // printf(), perror()
#include <string.h>      // memset()
#include <sys/socket.h>  // socket(), bind(), recvfrom(), sendto()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    // UDP 서버 소켓의 File Descriptor
    int server_fd;

    // 서버 주소 정보
    struct sockaddr_in server_addr;

    // 데이터를 보낸 클라이언트 주소 정보
    struct sockaddr_in client_addr;

    // client_addr 구조체 크기
    socklen_t client_len = sizeof(client_addr);

    // 데이터를 저장할 버퍼
    char buffer[BUFFER_SIZE];


    /*
     * 1. UDP Socket 생성
     *
     * AF_INET
     *      IPv4
     *
     * SOCK_DGRAM
     *      UDP
     *
     * TCP에서는 SOCK_STREAM을 사용했음
     */
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }


    /*
     * 2. 서버 주소 구조체 초기화
     */
    memset(&server_addr, 0, sizeof(server_addr));

    // IPv4
    server_addr.sin_family = AF_INET;

    // 8080 Port
    server_addr.sin_port = htons(PORT);

    // 모든 네트워크 인터페이스에서 받음
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    /*
     * 3. bind()
     *
     * UDP도 서버가 특정 Port에서
     * 데이터를 받으려면 bind()가 필요하다.
     *
     * server_fd
     *      ↓
     * Port 8080
     */
    if (bind(
            server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) == -1) {

        perror("bind");
        close(server_fd);
        return 1;
    }


    printf("UDP Server waiting on port %d...\n", PORT);


    /*
     * UDP는 TCP와 달리
     *
     * listen()
     * accept()
     *
     * 과정이 없다.
     *
     * 바로 recvfrom()으로 데이터를 받을 수 있다.
     */
    while (1)
    {
        /*
         * 4. recvfrom()
         *
         * 데이터를 받는다.
         *
         * 동시에
         * "누가 보냈는지"
         *
         * client_addr에 저장한다.
         */
        ssize_t len = recvfrom(
            server_fd,
            buffer,
            sizeof(buffer) - 1,
            0,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (len < 0) {
            perror("recvfrom");
            break;
        }


        // 문자열로 출력하기 위해 마지막에 '\0'
        buffer[len] = '\0';


        printf(
            "Received from %s:%d -> %s",
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port),
            buffer
        );


        /*
         * 5. sendto()
         *
         * 받은 데이터를
         * 데이터를 보낸 클라이언트에게 그대로 반환
         *
         * 그래서 Echo Server
         */
        ssize_t sent = sendto(
            server_fd,
            buffer,
            len,
            0,
            (struct sockaddr *)&client_addr,
            client_len
        );

        if (sent < 0) {
            perror("sendto");
            break;
        }
    }


    // UDP Socket 종료
    close(server_fd);

    return 0;
}