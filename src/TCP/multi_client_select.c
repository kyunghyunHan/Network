#include <arpa/inet.h>   // sockaddr_in, htons(), htonl(), inet_ntoa()
#include <stdio.h>       // printf(), perror()
#include <string.h>      // memset()
#include <sys/select.h>  // select(), fd_set, FD_ZERO, FD_SET, FD_ISSET
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), recv(), send()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    // 새로운 연결을 받는 서버 소켓
    int server_fd;

    // accept()가 반환하는 클라이언트 소켓
    int client_fd;

    // 서버 주소
    struct sockaddr_in server_addr;

    // 접속한 클라이언트 주소
    struct sockaddr_in client_addr;

    socklen_t client_len;

    // 데이터를 받을 버퍼
    char buffer[BUFFER_SIZE];


    /*
     * select()에서 사용할 FD 집합
     *
     * master_fds
     *   → 서버가 계속 감시해야 할 전체 FD
     *
     * read_fds
     *   → 이번 select()에서 읽을 준비가 된 FD 확인용
     */
    fd_set master_fds;
    fd_set read_fds;


    /*
     * 현재 사용 중인 가장 큰 FD 번호
     *
     * select()은
     *
     * 0 ~ max_fd
     *
     * 범위의 FD를 검사한다.
     */
    int max_fd;


    // --------------------------------------------------
    // 1. TCP Socket 생성
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

    // 모든 네트워크 인터페이스 허용
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


    printf("select TCP server started on port %d\n", PORT);


    // --------------------------------------------------
    // 5. FD 집합 초기화
    // --------------------------------------------------

    /*
     * FD_ZERO()
     *
     * FD 집합을 모두 비운다.
     */
    FD_ZERO(&master_fds);


    /*
     * server_fd를 감시 목록에 추가
     *
     * server_fd에 "읽기 이벤트"가 발생한다는 것은
     * 새로운 TCP 연결 요청이 있다는 의미다.
     */
    FD_SET(server_fd, &master_fds);


    /*
     * 처음에는 server_fd가 가장 큰 FD
     */
    max_fd = server_fd;


    // --------------------------------------------------
    // 6. 서버 메인 루프
    // --------------------------------------------------

    while (1)
    {
        /*
         * 중요!
         *
         * select()은 전달받은 fd_set 자체를 변경한다.
         *
         * 그래서 원본 master_fds를 바로 넘기면 안 되고
         * 매 반복마다 복사한다.
         */
        read_fds = master_fds;


        /*
         * select()
         *
         * 여러 File Descriptor 중
         * "읽을 준비가 된 FD"가 생길 때까지 기다린다.
         *
         * 첫 번째 인자:
         *
         * max_fd + 1
         *
         * Linux는 0부터 max_fd까지 검사한다.
         *
         * 두 번째:
         *
         * 읽기 이벤트를 감시할 FD 집합
         *
         * timeout = NULL
         *
         * 이벤트가 생길 때까지 무한 대기
         */
        int ready_count = select(
            max_fd + 1,
            &read_fds,
            NULL,
            NULL,
            NULL
        );

        if (ready_count == -1) {
            perror("select");
            break;
        }


        /*
         * 0부터 max_fd까지 확인하면서
         * 어떤 FD에 이벤트가 발생했는지 검사
         */
        for (int fd = 0; fd <= max_fd; fd++)
        {
            /*
             * FD_ISSET()
             *
             * 해당 FD가 이번에 준비되었는지 확인
             */
            if (!FD_ISSET(fd, &read_fds))
                continue;


            // ==========================================
            // server_fd에 이벤트 발생
            // ==========================================

            /*
             * server_fd가 readable 상태라는 것은
             *
             * 새로운 클라이언트 연결 요청이
             * 들어왔다는 의미다.
             */
            if (fd == server_fd)
            {
                client_len = sizeof(client_addr);

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


                /*
                 * 새 client_fd를
                 * 감시 목록에 추가
                 */
                FD_SET(client_fd, &master_fds);


                /*
                 * select()은 max_fd까지만 검사하므로
                 *
                 * 새 FD가 더 크다면 max_fd 갱신
                 */
                if (client_fd > max_fd)
                    max_fd = client_fd;
            }


            // ==========================================
            // Client FD에 이벤트 발생
            // ==========================================

            else
            {
                /*
                 * 클라이언트로부터 데이터 받기
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
                     * FD 감시 목록에서 제거
                     */
                    FD_CLR(fd, &master_fds);


                    /*
                     * Socket 종료
                     */
                    close(fd);
                }


                // --------------------------------------
                // recv 오류
                // --------------------------------------

                else if (len < 0)
                {
                    perror("recv");

                    FD_CLR(fd, &master_fds);

                    close(fd);
                }


                // --------------------------------------
                // 데이터 수신 성공
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
                     * 같은 클라이언트에게 그대로 반환
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


    close(server_fd);

    return 0;
}