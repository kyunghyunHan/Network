#include <arpa/inet.h>   // sockaddr_in, htons()
#include <stdio.h>       // printf(), perror()
#include <string.h>      // memset()
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), recv(), send()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 1024

int server(void)
{
    // 1. 서버가 클라이언트의 접속을 기다리는 소켓
    int server_fd;

    // 2. 실제 클라이언트와 통신할 소켓
    int client_fd;

    // 서버 주소 정보
    struct sockaddr_in server_addr;

    // 클라이언트 주소 정보
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    // 클라이언트가 보낸 데이터를 저장할 공간
    char buffer[BUFFER_SIZE];

    /*
     * socket()
     *
     * AF_INET     : IPv4
     * SOCK_STREAM : TCP
     * 0           : TCP에 맞는 기본 프로토콜 자동 선택
     *
     * 성공하면 File Descriptor 반환
     */
    /*
    0 → stdin (키보드 입력)
    1 → stdout (화면 출력)
    2 → stderr (에러 출력)
    3 → 우리가 만든 서버 소켓
    4 → 클라이언트 소켓
    */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // 구조체 내부를 전부 0으로 초기화
    memset(&server_addr, 0, sizeof(server_addr));

    // IPv4 사용
    server_addr.sin_family = AF_INET;

    /*
     * PORT 8080을 Network Byte Order로 변환
     *
     * h = host
     * to = to
     * n = network
     * s = short
     */
    server_addr.sin_port = htons(PORT);

    /*
     * 모든 네트워크 인터페이스에서 접속 허용
     *
     * 예:
     * 127.0.0.1
     * Wi-Fi IP
     * Ethernet IP
     */
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*
     * bind()
     *
     * 생성한 소켓에
     * "나는 8080 포트를 사용하겠다"
     * 라고 주소를 붙여준다.
     */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {

        perror("bind");
        close(server_fd);
        return 1;
    }

    /*
     * listen()
     *
     * 서버 소켓을
     * 클라이언트 연결 요청을 받을 수 있는 상태로 변경
     *
     * 5 = 연결 대기 큐 크기에 대한 힌트
     */
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("TCP Server waiting on port %d...\n", PORT);

    /*
     * accept()
     *
     * 클라이언트가 접속할 때까지 기다린다.
     *
     * 접속하면 새로운 FD를 반환한다.
     *
     * server_fd = 접속을 받는 역할
     * client_fd = 해당 클라이언트와 통신하는 역할
     */
    client_fd = accept(
        server_fd,
        (struct sockaddr *)&client_addr,
        &client_len
    );

    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    printf("Client connected!\n");

    while (1) {

        /*
         * recv()
         *
         * 클라이언트가 보내는 데이터를 기다린다.
         *
         * 반환값:
         * > 0 : 받은 바이트 수
         * = 0 : 클라이언트가 정상적으로 연결 종료
         * < 0 : 오류
         */
        ssize_t len = recv(
            client_fd,
            buffer,
            sizeof(buffer),
            0
        );

        if (len == 0) {
            printf("Client disconnected.\n");
            break;
        }

        if (len < 0) {
            perror("recv");
            break;
        }

        printf("Received %zd bytes\n", len);

        /*
         * Echo Server이므로
         * 받은 데이터를 그대로 클라이언트에게 보낸다.
         */
        ssize_t sent = send(
            client_fd,
            buffer,
            len,
            0
        );

        if (sent < 0) {
            perror("send");
            break;
        }
    }

    // 클라이언트 연결 종료
    close(client_fd);

    // 서버 소켓 종료
    close(server_fd);

    return 0;
}

/*

socket()
│
├─ TCP 소켓 하나 생성
│
▼
bind()
│
├─ "나는 8080번 포트를 사용할게."
│
▼
listen()
│
├─ "이제 접속을 받을 준비 완료."
│
▼
accept()
│
├─ "클라이언트가 오면 연결해 줘."
│
▼
recv()/send()
 */