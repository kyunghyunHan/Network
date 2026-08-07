# Network

---

# IP (Internet Protocol)

- 네트워크에서 컴퓨터를 구분하는 주소
- 데이터를 목적지까지 전달하는 역할
- 예)
  - 192.168.0.10
  - 8.8.8.8

---

# IPv4

- IP를 표현하는 방식 (Version 4)
- 32bit 주소 체계
- 4개의 숫자로 표현
- 각 숫자는 0~255

예)

192.168.219.110

Socket Programming

```c
socket(AF_INET, ...);
```

`AF_INET` = IPv4 사용

---

# IPv6

- IPv6는 IP를 표현하는 방식 (Version 6)
- 128bit 주소 체계
- IPv4 주소 부족 문제 해결

예)

2406:da18:1234:abcd:1234:5678:9abc:def0

Socket Programming

```c
socket(AF_INET6, ...);
```

---
## [TCP](./docs/tcp.md)

## [UDP](./docs/udp.md)
