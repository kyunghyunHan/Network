# OS
> Linux Driver, OS, System Programming 면접 대비 통합 노트

## 목차

1.  Linux 시스템 구조
2.  Hardware
3.  Device Driver
4.  Linux Kernel
5.  User Space(Application)
6.  System Call
7.  Application ↔ Hardware 동작 흐름
8.  Process
9.  Thread
10. Process vs Thread
11. Memory 구조
12. Stack vs Heap
13. Virtual Memory
14. Scheduler
15. Context Switch
16. Mutex
17. Semaphore
18. Race Condition
19. File Descriptor
20. fork()
21. exec()
22. pipe()
23. mmap()
24. Interrupt
25. Device Tree
26. Register Access
27. 면접 예상 질문
28. TSN LAB 공부 우선순위

------------------------------------------------------------------------

# 1. Linux 시스템 구조

``` text
+--------------------------------------------------+
|                Application (User Space)          |
|  TCP Server, Browser, Terminal, C Program        |
+--------------------------------------------------+
                    │
             System Call
                    │
                    ▼
+--------------------------------------------------+
|               Linux Kernel                       |
| Process Scheduler                                |
| Memory Manager                                   |
| File System                                      |
| Network Stack                                    |
| Device Driver                                    |
+--------------------------------------------------+
                    │
                    ▼
+--------------------------------------------------+
|                  Hardware                        |
| CPU, RAM, GPIO, UART, SPI, I2C, Ethernet         |
+--------------------------------------------------+
```

## 핵심

Application은 Hardware를 직접 제어하지 않는다.

``` text
Application
↓
System Call
↓
Kernel
↓
Driver
↓
Hardware
```

Hardware 이벤트는 반대로 전달된다.

``` text
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

------------------------------------------------------------------------

# 2. Hardware

-   CPU
-   RAM
-   GPIO
-   UART
-   SPI
-   I2C
-   Ethernet PHY
-   USB
-   SSD

하드웨어는 스스로 동작하지 않으며 반드시 Driver가 제어한다.

------------------------------------------------------------------------

# 3. Device Driver

역할

-   Hardware 초기화
-   Register 접근
-   Interrupt 처리
-   User Space와 Hardware 연결

예)

``` text
LED ON
↓
GPIO Register = 1
↓
LED ON
```

------------------------------------------------------------------------

# 4. Linux Kernel

관리 대상

-   Process
-   Thread
-   Memory
-   Scheduler
-   File System
-   Network Stack
-   Device Driver

------------------------------------------------------------------------

# 5. User Space(Application)

예)

``` c
printf("Hello");
socket();
read();
write();
```

Kernel에게 System Call을 통해 요청한다.

------------------------------------------------------------------------

# 6. System Call

대표 함수

``` c
open();
read();
write();
close();
ioctl();
fork();
exec();
mmap();
```

------------------------------------------------------------------------

# 7. Process

실행 중인 프로그램

특징

-   PID 보유
-   Heap
-   Stack
-   Register
-   File Descriptor 보유
-   다른 Process와 메모리 공유 안 함

------------------------------------------------------------------------

# 8. Thread

Process 내부 실행 단위

공유

-   Heap
-   Global 변수
-   File Descriptor

공유하지 않음

-   Stack
-   Register
-   Program Counter

------------------------------------------------------------------------

# 9. Process vs Thread

  Process         Thread
  --------------- -------------------
  독립 실행       Process 내부 실행
  메모리 공유 X   Heap 공유
  생성 비용 큼    생성 비용 작음
  안정성 높음     빠름

------------------------------------------------------------------------

# 10. Memory 구조

``` text
+----------------------+
| Text(Code)           |
+----------------------+
| Data(Global)         |
+----------------------+
| Heap                 |
|                      |
|                      |
| Stack                |
+----------------------+
```

### Text

프로그램 코드

### Data

전역 변수

### Heap

-   malloc()
-   calloc()
-   realloc()
-   free() 필요

### Stack

-   지역 변수
-   함수 종료 시 자동 제거

------------------------------------------------------------------------

# 11. Stack vs Heap

  Stack       Heap
  ----------- -----------------
  자동 관리   직접 관리
  빠름        상대적으로 느림
  지역 변수   malloc
  자동 삭제   free 필요

------------------------------------------------------------------------

# 12. Virtual Memory

``` text
Virtual Address
↓
MMU
↓
Physical Memory
```

장점

-   보호
-   충돌 방지
-   큰 메모리 공간 제공

------------------------------------------------------------------------

# 13. Scheduler

CPU 실행 순서를 결정한다.

------------------------------------------------------------------------

# 14. Context Switch

저장 대상

-   Register
-   Stack Pointer
-   Program Counter

------------------------------------------------------------------------

# 15. Mutex

동시에 하나만 접근 가능.

``` c
pthread_mutex_lock();
count++;
pthread_mutex_unlock();
```

------------------------------------------------------------------------

# 16. Semaphore

여러 Thread의 동시 접근 허용.

예)

Semaphore = 3 → 최대 3명 접근

------------------------------------------------------------------------

# 17. Race Condition

여러 Thread가 동시에 같은 데이터를 수정하는 문제.

해결

-   Mutex
-   Semaphore
-   Atomic

------------------------------------------------------------------------

# 18. File Descriptor

모든 것을 정수 번호(FD)로 관리한다.

``` c
int fd = open(...);
int sock = socket(...);
```

Socket도 FD이다.

------------------------------------------------------------------------

# 19. fork()

현재 Process를 복제한다.

``` text
Parent
↓
fork()
↓
Parent   Child
```

------------------------------------------------------------------------

# 20. exec()

현재 Process를 다른 프로그램으로 교체한다.

------------------------------------------------------------------------

# 21. pipe()

Process 간 통신.

예)

``` bash
ls | grep txt
```

------------------------------------------------------------------------

# 22. mmap()

파일을 메모리에 Mapping한다.

Shared Memory 구현에도 사용된다.

------------------------------------------------------------------------

# 23. Interrupt

``` text
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

------------------------------------------------------------------------

# 24. Device Tree

Linux가 Hardware 정보를 읽는 설정 파일.

------------------------------------------------------------------------

# 25. Register Access

Driver가 Register를 읽고 쓰며 Hardware를 제어한다.

------------------------------------------------------------------------

# 26. 면접 예상 질문

-   Process란?
-   Thread란?
-   Process와 Thread 차이?
-   Stack과 Heap 차이?
-   malloc은 어디에 저장되는가?
-   Race Condition이란?
-   Mutex와 Semaphore 차이?
-   Context Switch란?
-   File Descriptor란?
-   Socket도 FD인가?
-   fork()와 exec() 차이?
-   Driver 역할?
-   System Call이란?
-   Application이 Hardware를 직접 접근하지 않는 이유는?

------------------------------------------------------------------------

# 27. TSN LAB 공부 우선순위

★★★★★ - Linux 구조 - Process / Thread - Stack / Heap - System Call -
File Descriptor - Mutex - Semaphore - Interrupt

★★★★☆ - Scheduler - Context Switch - Virtual Memory - fork() - exec()

★★★☆☆ - pipe() - mmap() - Device Tree - Register Access

------------------------------------------------------------------------

# 핵심 한 줄

> **Application → System Call → Kernel → Driver → Hardware**

> **Hardware → Interrupt → Driver → Kernel → Application**
