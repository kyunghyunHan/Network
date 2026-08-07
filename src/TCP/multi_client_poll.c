#include <arpa/inet.h>   // sockaddr_in, htons(), htonl(), inet_ntoa()
#include <poll.h>        // poll(), struct pollfd
#include <stdio.h>       // printf(), perror()
#include <string.h>      // memset()
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), recv(), send()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

int main(void)
{
    // 새로운 연결을 받는 서버 소켓
    int server_fd;

    // accept()가 반환하는 클라이언트 소켓
    int client_fd;

    // 서버 주소
    struct sockaddr_in server_addr;

    // 클라이언트 주소
    struct sockaddr_in client_addr;

    socklen_t client_len;

    // 데이터 수신 버퍼
    char buffer[BUFFER_SIZE];

    /*
     * poll()이 감시할 FD 배열
     *
     * fds[0]     = server_fd
     * fds[1~]    = client_fd들
     */
    struct pollfd fds[MAX_CLIENTS + 1];

    /*
     * 현재 사용 중인 pollfd 개수
     *
     * 처음에는 server_fd 하나만 있으므로 1
     */
    int nfds = 1;


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


    printf("poll TCP server started on port %d\n", PORT);


    // --------------------------------------------------
    // 5. pollfd 배열 초기화
    // --------------------------------------------------

    /*
     * fds[0]에는 server_fd를 넣는다.
     *
     * server_fd에서 POLLIN 이벤트가 발생하면
     * 새로운 클라이언트 연결 요청이 있다는 뜻
     */
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;


    // --------------------------------------------------
    // 6. 메인 루프
    // --------------------------------------------------

    while (1)
    {
        /*
         * poll()
         *
         * fds
         *      감시할 pollfd 배열
         *
         * nfds
         *      감시할 FD 개수
         *
         * -1
         *      이벤트가 발생할 때까지 무한 대기
         */
        int ready_count = poll(fds, nfds, -1);

        if (ready_count == -1) {
            perror("poll");
            break;
        }


        // --------------------------------------------------
        // 7. server_fd 이벤트 확인
        // --------------------------------------------------

        /*
         * revents
         *
         * 실제로 발생한 이벤트가 저장되는 곳
         *
         * POLLIN
         *      읽을 데이터가 있거나
         *      서버 소켓이면 새로운 연결 요청이 있음
         */
        if (fds[0].revents & POLLIN)
        {
            client_len = sizeof(client_addr);

            client_fd = accept(
                server_fd,
                (struct sockaddr *)&client_addr,
                &client_len
            );

            if (client_fd == -1) {
                perror("accept");
            }
            else {

                printf(
                    "Client connected: %s:%d (fd=%d)\n",
                    inet_ntoa(client_addr.sin_addr),
                    ntohs(client_addr.sin_port),
                    client_fd
                );


                /*
                 * 새로운 client_fd를
                 * poll 배열에 추가
                 */
                if (nfds < MAX_CLIENTS + 1)
                {
                    fds[nfds].fd = client_fd;

                    /*
                     * 이 client_fd에
                     * 읽을 데이터가 들어오는지 감시
                     */
                    fds[nfds].events = POLLIN;

                    nfds++;
                }
                else
                {
                    printf("Too many clients\n");

                    close(client_fd);
                }
            }
        }


        // --------------------------------------------------
        // 8. Client FD 이벤트 확인
        // --------------------------------------------------

        /*
         * fds[0]은 server_fd이므로
         * Client는 index 1부터 검사
         */
        for (int i = 1; i < nfds; i++)
        {
            /*
             * POLLIN 이벤트가 없다면
             * 다음 FD로 넘어간다.
             */
            if (!(fds[i].revents & POLLIN))
                continue;


            // 현재 클라이언트 FD
            int fd = fds[i].fd;


            /*
             * 클라이언트 데이터 수신
             */
            ssize_t len = recv(
                fd,
                buffer,
                sizeof(buffer),
                0
            );


            // --------------------------------------------------
            // 클라이언트 연결 종료
            // --------------------------------------------------

            if (len == 0)
            {
                printf(
                    "Client disconnected (fd=%d)\n",
                    fd
                );

                close(fd);


                /*
                 * 연결 종료된 클라이언트를
                 * poll 배열에서 제거한다.
                 *
                 * 뒤의 요소들을 한 칸씩 앞으로 이동
                 */
                for (int j = i; j < nfds - 1; j++)
                {
                    fds[j] = fds[j + 1];
                }

                nfds--;

                /*
                 * 배열을 앞으로 당겼으므로
                 * 현재 i 위치에 새로운 FD가 들어왔다.
                 *
                 * 따라서 i-- 해서
                 * 다시 현재 위치부터 검사
                 */
                i--;
            }


            // --------------------------------------------------
            // recv 오류
            // --------------------------------------------------

            else if (len < 0)
            {
                perror("recv");

                close(fd);

                for (int j = i; j < nfds - 1; j++)
                {
                    fds[j] = fds[j + 1];
                }

                nfds--;

                i--;
            }


            // --------------------------------------------------
            // 정상적으로 데이터 수신
            // --------------------------------------------------

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
                 * 같은 클라이언트에게 그대로 보낸다.
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


    // --------------------------------------------------
    // 9. 열린 클라이언트 소켓 종료
    // --------------------------------------------------

    for (int i = 1; i < nfds; i++)
    {
        close(fds[i].fd);
    }

    close(server_fd);

    return 0;
}