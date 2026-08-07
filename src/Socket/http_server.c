#include <arpa/inet.h>   // sockaddr_in, htons(), htonl()
#include <stdio.h>       // printf(), perror(), snprintf()
#include <string.h>      // memset(), strlen()
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), recv(), send()
#include <unistd.h>      // close()

#define PORT 8080
#define BUFFER_SIZE 4096

int main(void)
{
    // 클라이언트 연결을 기다리는 서버 소켓
    int server_fd;

    // 실제 브라우저와 통신할 소켓
    int client_fd;

    // 서버 주소 정보
    struct sockaddr_in server_addr;

    // 클라이언트 주소 정보
    struct sockaddr_in client_addr;

    socklen_t client_len;

    // HTTP 요청을 저장할 버퍼
    char buffer[BUFFER_SIZE];


    // --------------------------------------------------
    // 1. TCP 소켓 생성
    // --------------------------------------------------

    /*
     * AF_INET
     *   → IPv4
     *
     * SOCK_STREAM
     *   → TCP
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }


    // --------------------------------------------------
    // 2. 서버 주소 설정
    // --------------------------------------------------

    memset(&server_addr, 0, sizeof(server_addr));

    // IPv4 사용
    server_addr.sin_family = AF_INET;

    // Port 8080
    server_addr.sin_port = htons(PORT);

    // 모든 네트워크 인터페이스에서 접속 허용
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    // --------------------------------------------------
    // 3. bind()
    // --------------------------------------------------

    /*
     * server_fd를 8080 Port에 연결
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


    // --------------------------------------------------
    // 4. listen()
    // --------------------------------------------------

    /*
     * TCP 연결 요청을 받을 준비
     */
    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }


    printf("HTTP Server started!\n");
    printf("Open browser: http://raspberrypi.local:%d\n", PORT);


    // --------------------------------------------------
    // 5. 서버 계속 실행
    // --------------------------------------------------

    while (1)
    {
        client_len = sizeof(client_addr);


        // --------------------------------------------------
        // 6. 브라우저 연결 기다림
        // --------------------------------------------------

        client_fd = accept(
            server_fd,
            (struct sockaddr *)&client_addr,
            &client_len
        );

        if (client_fd == -1) {
            perror("accept");
            continue;
        }


        printf("\nClient connected\n");


        // --------------------------------------------------
        // 7. HTTP 요청 받기
        // --------------------------------------------------

        /*
         * 브라우저가 대략 이런 요청을 보낸다.
         *
         * GET / HTTP/1.1
         * Host: raspberrypi.local:8080
         * User-Agent: ...
         */
        ssize_t len = recv(
            client_fd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (len <= 0)
        {
            if (len < 0)
                perror("recv");

            close(client_fd);
            continue;
        }


        // 문자열 끝 표시
        buffer[len] = '\0';


        // 브라우저가 실제로 보낸 요청 출력
        printf("----- HTTP REQUEST -----\n");
        printf("%s\n", buffer);
        printf("------------------------\n");


        // --------------------------------------------------
        // 8. HTML 작성
        // --------------------------------------------------

        const char *html =
            "<!DOCTYPE html>"
            "<html>"
            "<head>"
            "<meta charset=\"UTF-8\">"
            "<title>C HTTP Server</title>"
            "</head>"
            "<body>"
            "<h1>Hello Raspberry Pi!</h1>"
            "<p>This page is served by C.</p>"
            "</body>"
            "</html>";


        // --------------------------------------------------
        // 9. HTTP Header 작성
        // --------------------------------------------------

        char header[1024];

        int header_len = snprintf(
            header,
            sizeof(header),

            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",

            strlen(html)
        );


        // --------------------------------------------------
        // 10. HTTP Header 전송
        // --------------------------------------------------

        if (send(
                client_fd,
                header,
                header_len,
                0
            ) == -1)
        {
            perror("send header");
            close(client_fd);
            continue;
        }


        // --------------------------------------------------
        // 11. HTML 전송
        // --------------------------------------------------

        if (send(
                client_fd,
                html,
                strlen(html),
                0
            ) == -1)
        {
            perror("send html");
        }


        // --------------------------------------------------
        // 12. 브라우저 연결 종료
        // --------------------------------------------------

        close(client_fd);
    }


    close(server_fd);

    return 0;
}