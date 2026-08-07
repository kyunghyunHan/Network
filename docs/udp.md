# UDP (User Datagram Protocol)

- 데이터를 빠르게 전송하는 비연결형 프로토콜
- 연결(Connection)을 생성하지 않음
- 바로 데이터 전송
- 순서 보장 X
- 재전송 X
- ACK 없음
- 오버헤드가 작음
- 실시간 통신에 적합

## 장점

- 빠르다.
- 지연(Latency)이 적다.
- 구조가 단순하다.

## 단점

- 데이터 유실 가능
- 순서가 바뀔 수 있음
- 오류 복구 기능 없음
- 신뢰성 보장 X

## 사용 예

- 온라인 게임
- 실시간 스트리밍
- 음성 통화(VoIP)
- DNS
- DHCP

---

# UDP 동작 순서

```
socket()

↓

bind()

↓

recvfrom()

↓

sendto()

↓

close()
```

---

# Socket 생성

```c
socket(AF_INET, SOCK_DGRAM, 0);
```

- AF_INET → IPv4
- SOCK_DGRAM → UDP

---

# TCP와 가장 큰 차이

TCP

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
```

UDP

```
socket()

↓

bind()

↓

recvfrom()

↓

sendto()
```

### UDP에는 없는 것

- listen()
- accept()
- connect(일반적인 서버 기준)

왜?

→ 연결(Connection)을 만들지 않기 때문이다.

---

# recvfrom()

- 데이터를 받는다.
- 데이터를 보낸 상대(IP, Port)도 함께 알 수 있다.

---

# sendto()

- 특정 IP와 Port로 데이터를 전송한다.

---

# 면접 한 줄 정리

> UDP는 비연결형(Connectionless) 프로토콜로, 연결을 생성하지 않고 데이터를 전송합니다. ACK와 재전송 기능이 없어 신뢰성은 낮지만 오버헤드가 작고 지연이 적어 게임, DNS, 실시간 스트리밍 등에 주로 사용됩니다.