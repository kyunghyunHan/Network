#include <arpa/inet.h>   // sockaddr_in, htons(), htonl(), inet_ntoa()
#include <errno.h>       // errno, EAGAIN, EWOULDBLOCK
#include <fcntl.h>       // fcntl(), O_NONBLOCK
#include <stdio.h>       // printf(), perror()
#include <string.h>      // memset()
#include <sys/epoll.h>   // epoll_create1(), epoll_ctl(), epoll_wait()
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), recv(), send()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 100

// --------------------------------------------------
// FD를 Non-blocking 모드로 변경
// --------------------------------------------------
int set_nonblocking(int fd)
{
    // 현재 FD 상태 플래그 읽기
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        perror("fcntl F_GETFL");
        return -1;
    }

    // 기존 플래그 + O_NONBLOCK
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}

int main(void)
{
    int server_fd;
    int epoll_fd;

    struct sockaddr_in server_addr;

    char buffer[BUFFER_SIZE];

    // epoll에 FD 등록할 때 사용
    struct epoll_event event;

    // 실제 발생한 이벤트들이 들어옴
    struct epoll_event events[MAX_EVENTS];


    // --------------------------------------------------
    // 1. TCP Socket 생성
    // --------------------------------------------------

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }


    // --------------------------------------------------
    // 2. server_fd를 Non-blocking으로 변경
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

    if (listen(server_fd, 128) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("epoll + non-blocking server started on port %d\n", PORT);


    // --------------------------------------------------
    // 6. epoll 인스턴스 생성
    // --------------------------------------------------

    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(server_fd);
        return 1;
    }


    // --------------------------------------------------
    // 7. server_fd를 epoll에 등록
    // --------------------------------------------------

    memset(&event, 0, sizeof(event));

    /*
     * EPOLLIN
     * → 읽기 이벤트 감시
     *
     * server_fd에서 EPOLLIN이 발생하면
     * 새로운 클라이언트 연결 요청이 있다는 뜻
     */
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    if (epoll_ctl(
            epoll_fd,
            EPOLL_CTL_ADD,
            server_fd,
            &event
        ) == -1) {

        perror("epoll_ctl server_fd");
        close(epoll_fd);
        close(server_fd);
        return 1;
    }


    // --------------------------------------------------
    // 8. 메인 Event Loop
    // --------------------------------------------------

    while (1)
    {
        /*
         * 이벤트가 발생한 FD만 반환
         *
         * -1 = 이벤트가 생길 때까지 기다림
         */
        int event_count = epoll_wait(
            epoll_fd,
            events,
            MAX_EVENTS,
            -1
        );

        if (event_count == -1) {

            // Signal 때문에 깨진 경우 다시 대기
            if (errno == EINTR)
                continue;

            perror("epoll_wait");
            break;
        }


        // --------------------------------------------------
        // 발생한 이벤트 처리
        // --------------------------------------------------

        for (int i = 0; i < event_count; i++)
        {
            int fd = events[i].data.fd;


            // ==================================================
            // 새로운 Client 연결
            // ==================================================

            if (fd == server_fd)
            {
                /*
                 * Non-blocking server_fd에서는
                 * 연결 요청이 여러 개 쌓여 있을 수 있다.
                 *
                 * 따라서 accept()를 한 번만 하지 않고
                 * EAGAIN이 나올 때까지 반복한다.
                 */
                while (1)
                {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);

                    int client_fd = accept(
                        server_fd,
                        (struct sockaddr *)&client_addr,
                        &client_len
                    );


                    if (client_fd == -1)
                    {
                        /*
                         * 더 이상 accept할 연결이 없음
                         *
                         * Non-blocking에서는 정상적인 상황
                         */
                        if (errno == EAGAIN ||
                            errno == EWOULDBLOCK)
                        {
                            break;
                        }

                        perror("accept");
                        break;
                    }


                    printf(
                        "Client connected: %s:%d (fd=%d)\n",
                        inet_ntoa(client_addr.sin_addr),
                        ntohs(client_addr.sin_port),
                        client_fd
                    );


                    // 새 Client도 Non-blocking
                    if (set_nonblocking(client_fd) == -1)
                    {
                        close(client_fd);
                        continue;
                    }


                    /*
                     * client_fd를 epoll에 등록
                     */
                    memset(&event, 0, sizeof(event));

                    event.events = EPOLLIN;
                    event.data.fd = client_fd;


                    if (epoll_ctl(
                            epoll_fd,
                            EPOLL_CTL_ADD,
                            client_fd,
                            &event
                        ) == -1)
                    {
                        perror("epoll_ctl client");
                        close(client_fd);
                        continue;
                    }
                }
            }


            // ==================================================
            // Client 데이터 처리
            // ==================================================

            else
            {
                /*
                 * Non-blocking Socket이므로
                 * 읽을 수 있는 데이터가 여러 번 쌓여 있을 수 있다.
                 *
                 * EAGAIN이 나올 때까지 recv() 반복
                 */
                while (1)
                {
                    ssize_t len = recv(
                        fd,
                        buffer,
                        sizeof(buffer),
                        0
                    );


                    // ------------------------------------------
                    // 데이터 수신 성공
                    // ------------------------------------------

                    if (len > 0)
                    {
                        printf(
                            "Received %zd bytes from fd=%d\n",
                            len,
                            fd
                        );


                        /*
                         * Echo
                         *
                         * 주의:
                         * 실제 서버에서는 send()도
                         * 일부만 보낼 수 있으므로
                         * 출력 버퍼 관리가 필요하다.
                         *
                         * 여기서는 학습용으로 단순화.
                         */
                        ssize_t sent = send(
                            fd,
                            buffer,
                            len,
                            0
                        );


                        if (sent == -1)
                        {
                            if (errno == EAGAIN ||
                                errno == EWOULDBLOCK)
                            {
                                /*
                                 * 현재 Socket Send Buffer가 가득 참
                                 *
                                 * 실제 서버에서는
                                 * EPOLLOUT을 등록해서
                                 * 나중에 다시 전송한다.
                                 */
                                printf(
                                    "send would block (fd=%d)\n",
                                    fd
                                );
                            }
                            else
                            {
                                perror("send");
                            }
                        }
                    }


                    // ------------------------------------------
                    // Client 정상 종료
                    // ------------------------------------------

                    else if (len == 0)
                    {
                        printf(
                            "Client disconnected (fd=%d)\n",
                            fd
                        );

                        epoll_ctl(
                            epoll_fd,
                            EPOLL_CTL_DEL,
                            fd,
                            NULL
                        );

                        close(fd);

                        break;
                    }


                    // ------------------------------------------
                    // recv() == -1
                    // ------------------------------------------

                    else
                    {
                        /*
                         * 현재 읽을 데이터가 모두 소진됨
                         */
                        if (errno == EAGAIN ||
                            errno == EWOULDBLOCK)
                        {
                            break;
                        }

                        perror("recv");

                        epoll_ctl(
                            epoll_fd,
                            EPOLL_CTL_DEL,
                            fd,
                            NULL
                        );

                        close(fd);

                        break;
                    }
                }
            }
        }
    }


    close(epoll_fd);
    close(server_fd);

    return 0;
}