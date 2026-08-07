#include <arpa/inet.h>   // sockaddr_in, htons(), htonl(), inet_ntoa()
#include <errno.h>       // errno, EAGAIN, EWOULDBLOCK
#include <fcntl.h>       // fcntl(), O_NONBLOCK
#include <stdio.h>       // printf(), perror()
#include <string.h>      // memset()
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), recv(), send()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 1024

/*
 * 소켓을 Non-blocking 모드로 바꾸는 함수
 */
int set_nonblocking(int fd)
{
    /*
     * 현재 FD의 상태 플래그를 가져온다.
     */
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }

    /*
     * 기존 플래그에 O_NONBLOCK 추가
     */
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len;

    char buffer[BUFFER_SIZE];

    // --------------------------------------------------
    // 1. TCP 소켓 생성
    // --------------------------------------------------

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    // --------------------------------------------------
    // 2. server_fd를 Non-blocking으로 설정
    // --------------------------------------------------

    if (set_nonblocking(server_fd) == -1) {
        close(server_fd);
        return 1;
    }

    // --------------------------------------------------
    // 3. 서버 주소 설정
    // --------------------------------------------------

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // --------------------------------------------------
    // 4. bind()
    // --------------------------------------------------

    if (bind(
            server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) == -1) {

        perror("bind");
        close(server_fd);
        return 1;
    }

    // --------------------------------------------------
    // 5. listen()
    // --------------------------------------------------

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Non-blocking TCP server started on port %d\n", PORT);

    // --------------------------------------------------
    // 6. 메인 루프
    // --------------------------------------------------

    while (1)
    {
        client_len = sizeof(client_addr);

        /*
         * accept()
         *
         * Blocking Socket:
         * 클라이언트가 없으면 여기서 기다린다.
         *
         * Non-blocking Socket:
         * 클라이언트가 없으면 바로 -1 반환
         */
        client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (client_fd == -1)
        {
            /*
             * Non-blocking에서는
             * 연결할 클라이언트가 없으면
             *
             * errno = EAGAIN 또는 EWOULDBLOCK
             *
             * 이 될 수 있다.
             *
             * 이건 오류가 아니라
             * "지금은 연결이 없음"이라는 뜻이다.
             */
            if (errno != EAGAIN &&
                errno != EWOULDBLOCK)
            {
                perror("accept");
            }

            // 연결 없음 → 다시 루프
            continue;
        }

        printf(
            "Client connected: %s:%d (fd=%d)\n",
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port),
            client_fd
        );

        /*
         * 새 client_fd도 Non-blocking으로 변경
         */
        if (set_nonblocking(client_fd) == -1)
        {
            close(client_fd);
            continue;
        }

        // --------------------------------------------------
        // 7. 클라이언트 데이터 읽기
        // --------------------------------------------------

        while (1)
        {
            ssize_t len = recv(
                client_fd,
                buffer,
                sizeof(buffer),
                0
            );

            if (len > 0)
            {
                printf(
                    "Received %zd bytes from fd=%d\n",
                    len,
                    client_fd
                );

                /*
                 * 받은 데이터를 그대로 Echo
                 */
                ssize_t sent = send(
                    client_fd,
                    buffer,
                    len,
                    0
                );

                if (sent < 0)
                {
                    /*
                     * send()도 Non-blocking이라
                     * 지금 보낼 수 없는 상태일 수 있다.
                     */
                    if (errno == EAGAIN ||
                        errno == EWOULDBLOCK)
                    {
                        printf("send buffer is full\n");
                    }
                    else
                    {
                        perror("send");
                    }
                }
            }

            else if (len == 0)
            {
                /*
                 * 상대방이 연결 종료
                 */
                printf(
                    "Client disconnected (fd=%d)\n",
                    client_fd
                );

                break;
            }

            else
            {
                /*
                 * recv() == -1
                 *
                 * 데이터가 아직 없다면
                 * EAGAIN / EWOULDBLOCK
                 */
                if (errno == EAGAIN ||
                    errno == EWOULDBLOCK)
                {
                    /*
                     * 오류가 아니다.
                     *
                     * "지금 읽을 데이터가 없음"
                     */
                    continue;
                }

                perror("recv");
                break;
            }
        }

        close(client_fd);
    }

    close(server_fd);

    return 0;
}