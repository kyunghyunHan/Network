#include <arpa/inet.h>   // sockaddr_in, htons(), htonl(), inet_ntoa()
#include <stdio.h>       // printf(), perror()
#include <string.h>      // memset()
#include <sys/epoll.h>   // epoll_create1(), epoll_ctl(), epoll_wait()
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), recv(), send()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 100

int main(void)
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len;

    char buffer[BUFFER_SIZE];

    // epoll 인스턴스의 File Descriptor
    int epoll_fd;

    // epoll에 FD를 등록할 때 사용할 구조체
    struct epoll_event event;

    // epoll_wait()가 실제 발생한 이벤트들을 여기에 넣어준다.
    struct epoll_event events[MAX_EVENTS];


    // --------------------------------------------------
    // 1. TCP 서버 소켓 생성
    // --------------------------------------------------

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }


    // --------------------------------------------------
    // 2. 서버 주소 설정
    // --------------------------------------------------

    memset(&server_addr, 0, sizeof(server_addr));

    // IPv4
    server_addr.sin_family = AF_INET;

    // Port 8080
    server_addr.sin_port = htons(PORT);

    // 모든 네트워크 인터페이스에서 접속 허용
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    // --------------------------------------------------
    // 3. bind()
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
    // 4. listen()
    // --------------------------------------------------

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("epoll TCP server started on port %d\n", PORT);


    // --------------------------------------------------
    // 5. epoll 인스턴스 생성
    // --------------------------------------------------

    /*
     * epoll_create1()
     *
     * epoll을 관리하기 위한 객체를 커널에 생성한다.
     *
     * 성공하면 epoll_fd 반환
     */
    epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        perror("epoll_create1");
        close(server_fd);
        return 1;
    }


    // --------------------------------------------------
    // 6. server_fd를 epoll에 등록
    // --------------------------------------------------

    memset(&event, 0, sizeof(event));

    /*
     * EPOLLIN
     *
     * 읽을 수 있는 이벤트를 감시한다.
     *
     * server_fd가 읽을 수 있다는 것은
     * 새로운 클라이언트 연결 요청이 있다는 의미
     */
    event.events = EPOLLIN;

    /*
     * 이벤트가 발생했을 때
     * 어떤 FD인지 알기 위해 저장
     */
    event.data.fd = server_fd;


    /*
     * epoll_ctl()
     *
     * EPOLL_CTL_ADD
     *      새로운 FD를 epoll 감시 목록에 추가
     */
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
    // 7. 서버 메인 루프
    // --------------------------------------------------

    while (1)
    {
        /*
         * epoll_wait()
         *
         * 이벤트가 발생할 때까지 기다린다.
         *
         * epoll_fd
         *      감시할 epoll 인스턴스
         *
         * events
         *      발생한 이벤트들을 저장할 배열
         *
         * MAX_EVENTS
         *      한 번에 받을 최대 이벤트 수
         *
         * -1
         *      이벤트가 발생할 때까지 무한 대기
         */
        int event_count = epoll_wait(
            epoll_fd,
            events,
            MAX_EVENTS,
            -1
        );

        if (event_count == -1) {
            perror("epoll_wait");
            break;
        }


        // --------------------------------------------------
        // 8. 실제 발생한 이벤트만 처리
        // --------------------------------------------------

        for (int i = 0; i < event_count; i++)
        {
            // 이벤트가 발생한 FD
            int fd = events[i].data.fd;


            // ==========================================
            // server_fd 이벤트
            // ==========================================

            if (fd == server_fd)
            {
                client_len = sizeof(client_addr);

                /*
                 * 새로운 클라이언트 연결 수락
                 */
                client_fd = accept(
                    server_fd,
                    (struct sockaddr *)&client_addr,
                    &client_len
                );

                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }


                printf(
                    "Client connected: %s:%d (fd=%d)\n",
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port),
                    client_fd
                );


                // 새 client_fd를 epoll에 등록
                memset(&event, 0, sizeof(event));

                event.events = EPOLLIN;

                event.data.fd = client_fd;


                if (epoll_ctl(
                        epoll_fd,
                        EPOLL_CTL_ADD,
                        client_fd,
                        &event
                    ) == -1) {

                    perror("epoll_ctl client_fd");
                    close(client_fd);
                    continue;
                }
            }


            // ==========================================
            // Client FD 이벤트
            // ==========================================

            else
            {
                /*
                 * 클라이언트 데이터 수신
                 */
                ssize_t len = recv(
                    fd,
                    buffer,
                    sizeof(buffer),
                    0
                );


                // --------------------------------------
                // 클라이언트 연결 종료
                // --------------------------------------

                if (len == 0)
                {
                    printf(
                        "Client disconnected (fd=%d)\n",
                        fd
                    );


                    /*
                     * epoll 감시 목록에서 제거
                     */
                    epoll_ctl(
                        epoll_fd,
                        EPOLL_CTL_DEL,
                        fd,
                        NULL
                    );


                    close(fd);
                }


                // --------------------------------------
                // recv 오류
                // --------------------------------------

                else if (len < 0)
                {
                    perror("recv");

                    epoll_ctl(
                        epoll_fd,
                        EPOLL_CTL_DEL,
                        fd,
                        NULL
                    );

                    close(fd);
                }


                // --------------------------------------
                // 정상 데이터 수신
                // --------------------------------------

                else
                {
                    printf(
                        "Received %zd bytes from fd=%d\n",
                        len,
                        fd
                    );


                    /*
                     * Echo Server
                     *
                     * 받은 데이터를
                     * 같은 클라이언트에게 그대로 전송
                     */
                    ssize_t sent = send(
                        fd,
                        buffer,
                        len,
                        0
                    );

                    if (sent < 0) {
                        perror("send");
                    }
                }
            }
        }
    }


    // --------------------------------------------------
    // 9. 종료
    // --------------------------------------------------

    close(epoll_fd);
    close(server_fd);

    return 0;
}