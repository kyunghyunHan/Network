# OS

---

# 목차

1. Process
2. Thread
3. Process vs Thread
4. Memory Management
5. Stack vs Heap
6. Virtual Memory
7. Scheduler
8. Context Switch
9. Mutex
10. Semaphore
11. Race Condition
12. File Descriptor
13. fork()
14. exec()
15. pipe()
16. mmap()
17. 면접 예상 질문

---

# 1. Process (프로세스)

## 정의

실행 중인 프로그램.

예를 들어

```
Chrome
VSCode
TCP Server
```

모두 각각 하나의 Process이다.

프로세스는 다음 정보를 가진다.

- PID(Process ID)
- 자신만의 메모리
- 열린 파일(File Descriptor)
- Stack
- Heap
- CPU 상태(Register)

---

## 특징

- 다른 Process와 메모리를 공유하지 않는다.
- 운영체제가 각각 독립적으로 관리한다.

예)

```
Process A

Heap

Stack


Process B

Heap

Stack
```

둘은 서로 접근할 수 없다.

---

# 2. Thread (쓰레드)

## 정의

프로세스 내부에서 실제 실행되는 작업 단위.

예)

```
TCP Server(Process)

├── Thread 1
├── Thread 2
└── Thread 3
```

---

## 공유하는 것

- Heap
- Global 변수
- File Descriptor

## 공유하지 않는 것

- Stack
- Register
- Program Counter

---

# 3. Process vs Thread

| Process | Thread |
|----------|--------|
| 독립 실행 | 프로세스 내부 실행 |
| 메모리 공유 안 함 | 메모리 공유 |
| 생성 비용 큼 | 생성 비용 적음 |
| 안정성 높음 | 빠름 |

---

# 4. Memory Management

프로그램이 실행되면 메모리는 다음과 같이 구성된다.

```
+----------------------+
| Text(Code)           |
+----------------------+
| Data(Global)         |
+----------------------+
| Heap (malloc)        |
| ↑                    |
|                      |
|                      |
| ↓                    |
| Stack (Local)        |
+----------------------+
```

---

## Text

프로그램 코드

```
int main(){}
```

---

## Data

전역 변수

```
int count = 0;
```

---

## Heap

동적 메모리

```
malloc()

calloc()

realloc()
```

개발자가 직접 관리해야 한다.

```
free(ptr);
```

---

## Stack

지역 변수

```
void func()
{
    int x = 10;
}
```

함수가 끝나면 자동 삭제된다.

---

# 5. Stack vs Heap

| Stack | Heap |
|--------|------|
| 자동 관리 | 직접 관리 |
| 빠름 | 느림 |
| 지역 변수 | malloc |
| 함수 종료 시 삭제 | free 필요 |

---

# 6. Virtual Memory

모든 프로세스는 자기만의 메모리를 가진 것처럼 보인다.

실제로는

```
Virtual Address

↓

MMU

↓

Physical Memory
```

운영체제가 주소를 변환한다.

장점

- 메모리 보호
- 충돌 방지
- 큰 메모리 사용 가능

---

# 7. Scheduler

CPU는 하나지만

```
Thread A

↓

Thread B

↓

Thread C
```

매우 빠르게 번갈아 실행한다.

이 작업을 Scheduler가 수행한다.

---

# 8. Context Switch

CPU가 실행 대상을 바꾸는 과정

```
Thread A

↓

Thread B
```

이때 저장하는 것

- Register
- Stack Pointer
- Program Counter

비용이 존재한다.

---

# 9. Mutex

Mutual Exclusion

동시에 하나만 접근 가능

예)

```
count++;
```

두 Thread가 동시에 접근하면 문제가 생긴다.

Mutex 사용

```c
pthread_mutex_lock();

count++;

pthread_mutex_unlock();
```

---

# 10. Semaphore

Mutex와 비슷하지만

동시에 여러 개 허용 가능

예)

프린터 3대

```
Semaphore = 3
```

3명까지 동시에 사용 가능.

---

# 11. Race Condition

두 Thread가 동시에 같은 데이터를 수정하는 문제.

예)

```
count++;

count++;
```

원래 결과

```
2
```

실제로

```
1
```

이 될 수도 있다.

해결

- Mutex
- Semaphore
- Atomic

---

# 12. File Descriptor

리눅스에서는 거의 모든 것이 File이다.

```
File

Socket

Pipe

Terminal

USB
```

모두 File Descriptor(FD)로 관리한다.

예)

```c
int fd = open(...);

int sock = socket(...);
```

둘 다 FD이다.

---

# 13. fork()

새로운 Process 생성

```
Parent

↓

fork()

↓

Parent

Child
```

예)

```c
pid_t pid = fork();
```

pid == 0

→ Child

pid > 0

→ Parent

---

# 14. exec()

현재 Process를 다른 프로그램으로 교체

```
현재

my_program

↓

exec()

↓

ls
```

예)

```c
execl("/bin/ls","ls",NULL);
```

---

# 15. pipe()

Process 간 통신

```
Parent

↓

Pipe

↓

Child
```

쉘에서

```
ls | grep txt
```

도 Pipe이다.

---

# 16. mmap()

파일을 메모리에 연결

```
File

↓

Memory Mapping

↓

Pointer
```

예)

```c
char *p = mmap(...);

printf("%c", p[0]);
```

또는

Shared Memory 구현에도 사용한다.

---

# 면접에서 자주 나오는 질문

### Process란?

실행 중인 프로그램이다.

---

### Thread란?

프로세스 내부의 실행 단위이다.

---

### Process와 Thread 차이는?

Process는 메모리를 공유하지 않는다.

Thread는 Heap과 전역 변수를 공유한다.

---

### Stack과 Heap 차이는?

Stack은 자동 관리

Heap은 malloc/free로 직접 관리한다.

---

### malloc은 어디에 저장되는가?

Heap

---

### 지역 변수는 어디에 저장되는가?

Stack

---

### Race Condition이란?

여러 Thread가 동시에 같은 데이터를 수정하는 문제.

---

### Mutex란?

한 번에 하나의 Thread만 접근하도록 하는 동기화 객체.

---

### Semaphore란?

여러 개의 Thread 접근을 허용하는 동기화 객체.

---

### Context Switch란?

CPU가 다른 Process 또는 Thread로 실행을 전환하는 과정.

---

### File Descriptor란?

리눅스에서 파일, 소켓 등을 관리하는 정수 번호.

---

### socket도 File Descriptor인가?

그렇다.

```
int sock = socket(...);
```

sock도 FD이다.

---

### fork()란?

현재 Process를 복제하여 Child Process를 만든다.

---

### exec()란?

현재 Process를 새로운 프로그램으로 교체한다.

---

# 공부 우선순위

⭐⭐⭐⭐⭐

- Process
- Thread
- Stack / Heap
- Mutex
- Semaphore
- File Descriptor

⭐⭐⭐⭐

- Context Switch
- Scheduler
- Virtual Memory
- fork()
- exec()

⭐⭐⭐

- pipe()
- mmap()

```

이 정도를 이해하고 C 코드로 간단한 예제를 작성할 수 있다면 **TSN LAB의 "기본적인 OS 지식(쓰레드, 메모리, 프로세스 관리 방법 등)" 요구사항은 충분히 대비할 수 있다.**