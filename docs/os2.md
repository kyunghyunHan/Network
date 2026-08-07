# Linux 시스템 전체 구조 (TSN LAB 면접 대비)

# 전체 구조

리눅스 시스템은 크게 아래와 같이 구성된다.

```
+--------------------------------------------------+
|                Application (User Space)          |
|  TCP Server, Browser, Terminal, C Program        |
+--------------------------------------------------+
                    │
                    │ System Call
                    ▼
+--------------------------------------------------+
|               Linux Kernel (Kernel Space)        |
|                                                  |
| Process Scheduler                                |
| Memory Manager                                   |
| File System                                      |
| Network Stack                                    |
| Device Driver                                    |
+--------------------------------------------------+
                    │
                    ▼
+--------------------------------------------------+
|                Hardware                          |
| CPU, RAM, GPIO, UART, SPI, I2C, Ethernet, USB    |
+--------------------------------------------------+
```

---

# 1. Hardware

실제 물리적인 장치이다.

예)

- CPU
- RAM
- GPIO
- LED
- Button
- UART
- SPI
- I2C
- Ethernet PHY
- SSD

하드웨어는 스스로 동작하지 않는다.

반드시 드라이버가 제어해야 한다.

---

# 2. Device Driver

드라이버는

**하드웨어를 제어하는 소프트웨어**이다.

예)

LED Driver

```
LED ON

↓

GPIO Register = 1

↓

LED 켜짐
```

Application은 GPIO Register를 직접 만지지 않는다.

Driver가 대신 제어한다.

---

## Driver의 역할

- Hardware 초기화
- Register 접근
- Interrupt 처리
- User Space와 Hardware 연결

---

# 3. Linux Kernel

커널은 운영체제의 핵심이다.

Application과 Hardware 사이에서 모든 자원을 관리한다.

```
Application

↓

Kernel

↓

Hardware
```

Kernel이 관리하는 것

- Process
- Thread
- Memory
- Scheduler
- File System
- Network
- Device Driver

---

# 4. Application (User Space)

우리가 작성하는 프로그램이다.

예)

```c
printf("Hello");

socket();

write();

read();
```

Application은 Hardware를 직접 건드리지 않는다.

항상 Kernel에게 요청한다.

---

# 5. System Call

Application이 Kernel에게 요청하는 방법.

예)

```
Application

↓

write()

↓

Kernel

↓

Driver

↓

LED
```

대표적인 System Call

```
open()

read()

write()

close()

ioctl()

fork()

exec()

mmap()
```

---

# 6. Application → Hardware까지의 흐름

예)

LED 켜기

```
write(fd,"1",1);

↓

System Call

↓

Kernel

↓

LED Driver

↓

GPIO Register

↓

LED ON
```

---

# 7. Hardware → Application까지의 흐름

예)

버튼을 누른 경우

```
Button

↓

Interrupt

↓

Driver

↓

Kernel

↓

Application
```

---

# 8. Process

실행 중인 프로그램

예)

```
Chrome

VSCode

TCP Server
```

모두 각각 Process이다.

각 Process는

- PID
- Stack
- Heap
- 열린 파일
- Register

를 가진다.

---

## 특징

Process끼리는

메모리를 공유하지 않는다.

```
Process A

Heap

Stack



Process B

Heap

Stack
```

독립적이다.

---

# 9. Thread

Process 내부의 실행 단위

예)

```
TCP Server

Thread 1

Thread 2

Thread 3
```

공유하는 것

- Heap
- Global Variable
- File Descriptor

공유하지 않는 것

- Stack
- Register

---

# 10. Memory

프로그램이 실행되면

```
+------------------+
| Text             |
+------------------+
| Data             |
+------------------+
| Heap             |
|                  |
|                  |
|                  |
| Stack            |
+------------------+
```

---

## Text

프로그램 코드

---

## Data

전역 변수

---

## Heap

malloc()

calloc()

realloc()

사용

직접 free 해야 한다.

---

## Stack

지역 변수

함수가 끝나면 자동 삭제

---

# 11. Virtual Memory

프로세스마다

자기만의 메모리를 사용하는 것처럼 보인다.

실제로는

```
Virtual Address

↓

MMU

↓

Physical Memory
```

장점

- 보호
- 충돌 방지
- 큰 메모리 사용

---

# 12. Scheduler

CPU는 하나지만

```
Thread A

↓

Thread B

↓

Thread C
```

빠르게 번갈아 실행한다.

이것을 Scheduler가 담당한다.

---

# 13. Context Switch

CPU가

```
Thread A

↓

Thread B
```

로 바뀌는 과정

저장하는 것

- Register
- Stack Pointer
- Program Counter

---

# 14. Mutex

Thread가 동시에 접근하지 못하도록 막는다.

```
Thread A

↓

Mutex

↓

Shared Data
```

---

# 15. Semaphore

Mutex와 비슷하지만

동시에 여러 개 허용 가능

예)

Printer 3대

Semaphore = 3

---

# 16. Race Condition

두 Thread가 동시에

```
count++;
```

를 실행하면

원하지 않는 결과가 발생할 수 있다.

Mutex로 해결한다.

---

# 17. File Descriptor

리눅스에서는

거의 모든 것이 File이다.

예)

```
File

Socket

Pipe

Terminal

USB
```

모두 FD(File Descriptor)로 관리된다.

예)

```c
int fd = open(...);

int sock = socket(...);
```

둘 다 FD이다.

---

# 18. fork()

현재 Process를 복제한다.

```
Parent

↓

fork()

↓

Parent

Child
```

---

# 19. exec()

현재 Process를

다른 프로그램으로 교체한다.

```
현재 Program

↓

exec()

↓

ls
```

---

# 20. pipe()

Process 간 통신

```
Parent

↓

Pipe

↓

Child
```

쉘의

```
ls | grep txt
```

도 Pipe이다.

---

# 21. mmap()

파일을 메모리에 연결한다.

```
File

↓

Memory Mapping

↓

Pointer
```

Shared Memory 구현에도 사용된다.

---

# 22. Linux Driver에서 가장 중요한 흐름

```
Application

↓

System Call

↓

Kernel

↓

Device Driver

↓

Hardware
```

그리고

```
Hardware

↓

Interrupt

↓

Driver

↓

Kernel

↓

Application
```

이 두 흐름을 이해하면 Linux Driver의 큰 구조를 이해한 것이다.

---

# TSN LAB 기준 공부 우선순위

★★★★★

- Hardware ↔ Driver ↔ Kernel ↔ Application
- Process
- Thread
- Stack / Heap
- System Call
- File Descriptor
- Mutex
- Semaphore

★★★★☆

- Scheduler
- Context Switch
- Virtual Memory
- fork()
- exec()

★★★☆☆

- pipe()
- mmap()
- Interrupt
- Device Tree
- Register Access

---

# 핵심 한 줄 정리

Application은 Hardware를 직접 제어하지 않는다.

항상

Application

→ System Call

→ Kernel

→ Driver

→ Hardware

순서로 동작하며,

Hardware의 이벤트는

Hardware

→ Driver

→ Kernel

→ Application

순서로 전달된다.

이 구조를 이해하는 것이 Linux Driver와 기본 OS 지식의 핵심이다.