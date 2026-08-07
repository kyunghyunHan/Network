# TCP
- 신뢰성 있게 데이터를 전송하는 연결형 protocol
- 연결은 먼저 (-WAY Handshake)
- data 순서 보장
- 상대방이 받았는지 확인
- 오류 검사
- 흐름제어
- 느리다
- 파일전송
- 순서를 맞춰줌
## 단점
- 연결해야함
- ACK확인
- 재전송

# USE

- HTTP
- HTTPS


# TCP

- 연결(Connection)을 먼저 생성
- 신뢰성 보장
- 순서 보장
- 재전송 지원
- ACK 사용
- HTTP, HTTPS, SSH 등에서 사용

Socket

```c
socket(AF_INET, SOCK_STREAM, 0);
```

---

# UDP

- 연결 없이 데이터 전송
- 빠름
- 순서 보장 X
- 재전송 X
- ACK 없음
- 게임, 스트리밍, DNS 등에 사용

Socket

```c
socket(AF_INET, SOCK_DGRAM, 0);
```

---

# TCP vs UDP

| TCP | UDP |
|------|------|
| 연결 O | 연결 X |
| 신뢰성 O | 신뢰성 X |
| 순서 보장 | 순서 보장 X |
| 재전송 | 재전송 X |
| ACK 사용 | ACK 없음 |
| 느림 | 빠름 |

---

# 3-Way Handshake

TCP 연결 생성 과정

```
Client                     Server

SYN ---------------------->

        <------------- SYN + ACK

ACK ----------------------->
```

목적

- 서로 통신 가능한지 확인
- 초기 Sequence Number 교환
- 연결(Connection) 생성

---

# Sequence Number

- TCP는 데이터를 **Byte 단위**로 번호를 붙인다.
- 순서 보장
- 유실 확인
- 재전송에 사용

예)

```
Hello

H = 1000
e = 1001
l = 1002
l = 1003
o = 1004
```

---

# ACK (Acknowledgment)

- 데이터를 정상적으로 받았음을 알리는 응답
- 다음에 받고 싶은 Sequence Number를 의미

예)

```
SEQ = 1000
Length = 500 Bytes

↓

ACK = 1500
```

의미

```
1499 Byte까지 받았습니다.
1500부터 보내주세요.
```

---

# Sliding Window

- 여러 개의 패킷을 한 번에 전송
- ACK를 기다리지 않고 연속 전송 가능
- TCP 성능 향상

---

# Flow Control

- 송신 속도를 수신자가 처리 가능한 속도로 조절
- Receiver의 Buffer Overflow 방지
- Window Size 사용

---

# Congestion Control

- 네트워크 혼잡 시 전송 속도를 줄이는 기능
- 인터넷 전체의 혼잡을 방지

---

# Retransmission

- ACK가 일정 시간 오지 않으면
- 데이터가 유실되었다고 판단
- 해당 데이터를 다시 전송

---

# 4-Way Termination

TCP 연결 종료 과정

```
Client                     Server

FIN ---------------------->

        <------------- ACK

        <------------- FIN

ACK ----------------------->
```

목적

- 양쪽이 각각 연결 종료를 확인

---

# TIME_WAIT

- 마지막 ACK를 보낸 후 일정 시간 대기
- 마지막 ACK가 유실될 경우를 대비
- 동일한 연결이 즉시 재사용되는 것을 방지

---

# Socket API

```
socket()

↓

bind()

↓

listen()

↓

accept()

↓

recv()

↓

send()

↓

close()
```

Client

```
socket()

↓

connect()

↓

send()

↓

recv()

↓

close()
```



---



# IP (Internet Protocol)

- 네트워크에서 장치를 구분하기 위한 주소
- 데이터를 목적지까지 전달하는 역할
- 데이터의 도착, 순서, 재전송은 보장하지 않음

예)

```
192.168.0.10
```

---

# IPv4

- 현재 가장 많이 사용하는 IP 주소 체계
- 32bit 주소
- 4개의 숫자로 표현
- 각 숫자는 0 ~ 255

예)

```
192.168.0.10
```

Socket

```c
AF_INET
```

---

# IPv6

- IPv4 주소 부족 문제를 해결하기 위한 새로운 IP 주소 체계
- 128bit 주소
- IPv4보다 훨씬 많은 주소 사용 가능

예)

```
2001:db8::1
```

Socket

```c
AF_INET6
```

---

# Port

- 하나의 컴퓨터 안에서 프로그램을 구분하는 번호
- IP는 컴퓨터를 찾고, Port는 프로그램을 찾는다.

예)

```
192.168.0.10:8080
```

대표 Port

| Port | Service |
|------|----------|
| 22 | SSH |
| 53 | DNS |
| 80 | HTTP |
| 443 | HTTPS |

---

# DNS (Domain Name System)

- 도메인 이름을 IP 주소로 변환하는 시스템

예)

```
google.com

↓

142.250.xxx.xxx
```

사용자는 도메인을 사용하지만 실제 네트워크 통신은 IP 주소를 이용한다.

---

# ARP (Address Resolution Protocol)

- 같은 LAN(Local Area Network)에서 IP 주소를 MAC 주소로 변환하는 프로토콜

예)

```
192.168.0.10

↓

AA:BB:CC:DD:EE:FF
```

동작 과정

```
IP

↓

ARP Request

↓

ARP Reply

↓

MAC Address 확인
```

---

# HTTP (HyperText Transfer Protocol)

- 웹 브라우저와 웹 서버가 통신하는 응용 계층 프로토콜
- TCP 기반
- 기본 Port : 80
- 암호화되지 않음

대표 Method

- GET
- POST
- PUT
- DELETE

예)

```http
GET / HTTP/1.1
Host: example.com
```

응답

```http
HTTP/1.1 200 OK
```

---

# HTTPS (HyperText Transfer Protocol Secure)

- HTTP + TLS(SSL)
- HTTP 데이터를 암호화하여 전송
- TCP 기반 (HTTP/1.1, HTTP/2)
- 기본 Port : 443

장점

- 데이터 암호화
- 서버 인증
- 데이터 위변조 방지

---

# HTTP vs HTTPS

| HTTP | HTTPS |
|------|--------|
| 암호화 X | 암호화 O |
| Port 80 | Port 443 |
| 평문 전송 | TLS 암호화 |
| 보안 낮음 | 보안 높음 |

---

# 브라우저 접속 과정

```
URL 입력

↓

DNS
(도메인을 IP로 변환)

↓

ARP
(IP를 MAC으로 변환)

↓

TCP
(3-Way Handshake)

↓

HTTP Request

↓

HTTP Response
```

---

# 면접 핵심 정리

### IP

- 네트워크에서 장치를 구분하는 주소

### IPv4

- 32bit IP 주소 체계

### IPv6

- 128bit IP 주소 체계

### Port

- 컴퓨터 안에서 프로그램을 구분하는 번호

### DNS

- 도메인을 IP 주소로 변환

### ARP

- IP 주소를 MAC 주소로 변환

### HTTP

- 웹 통신 프로토콜
- TCP 사용
- Port 80

### HTTPS

- TLS로 암호화된 HTTP
- TCP 사용(HTTP/1.1, HTTP/2)
- Port 443