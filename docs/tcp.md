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