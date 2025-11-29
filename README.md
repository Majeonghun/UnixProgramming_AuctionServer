# UnixProgramming_AuctionServer
Unix Programming Project: Auction Server Implementation

# Unix 경매 시스템

## 개요
이 프로젝트는 **Unix 기반의 명령어 경매 시스템**으로, **C 언어**와 **System V 메시지 큐**, **POSIX 스레드**를 이용하여 구현되었습니다.  
여러 클라이언트가 동시에 경매에 참여할 수 있으며, 서버가 라운드별 입찰을 관리하고 최고 입찰자를 결정하는 방식입니다.

---

## 프로젝트 구조
/AuctionSystem
├── server.c # 서버 프로그램: 경매 로직 및 UI 출력
├── client.c # 클라이언트 프로그램: 입찰 입력
├── README.md # 프로젝트 설명

---

## 주요 기능

### 서버(Master)
- 경매 아이템 **이름과 설명** 입력
- 여러 클라이언트 관리 (최대 10명)
- **경매 라운드 관리**:
  - 1라운드: 모든 클라이언트 입찰 초기화
  - 2라운드 이후: 이전 라운드 입찰값 유지, 새로 입력한 값만 덮어쓰기
- **최고 입찰자(Client ID 기준) 표시**
- 라운드별 입찰 테이블 출력
- **스레드와 뮤텍스**로 안전하게 입찰값 관리
- **30초 라운드** (코드에서 변경 가능)

### 클라이언트(Client)
- 서버 메시지 큐에 연결
- 언제든지 자유롭게 입찰 가능
- 각 입찰은 **Client ID 기반**으로 전송
- 입력 검증 및 오류 처리
- 마지막으로 입력한 값이 **해당 라운드 최종 입찰값**으로 서버에 적용

---

## 경매 규칙
1. 서버는 **master**, 클라이언트는 **participant** 역할
2. 서버에서 아이템 이름과 설명 등록
3. 클라이언트는 언제든 입찰 가능
4. 입찰 메시지 형식: `<ClientID> <BidAmount>`
5. 각 라운드 종료 후(30초):
   - 서버가 **모든 클라이언트 입찰** 출력
   - **최고 입찰자가 단독**인지 확인
   - 단독 최고 입찰자가 없으면 다음 라운드 진행, **이전 입찰값 유지**
6. 경매 종료 조건:
   - 단독 최고 입찰자가 연속 라운드에서 유지될 경우
   - 서버에서 최종 **낙찰자와 금액** 출력

---

## 구현 특징

- **메시지 큐 통신**
  - System V 메시지 큐를 사용하여 서버와 클라이언트 간 통신
  - 입찰 메시지와 낙찰 메시지 구분 가능
- **스레드 활용**
  - 서버는 **수신 스레드**를 통해 입찰 메시지를 지속적으로 수신
  - 메인 스레드는 라운드 진행과 UI 출력 담당
- **입찰 관리**
  - 클라이언트별 입찰은 `clients[]` 배열에 저장
  - 뮤텍스(`pthread_mutex_t`)로 동시성 문제 방지
- **C89 호환**
  - `for`문 변수 외부 선언
  - 표준 Unix 컴파일러에서 C99 옵션 없이 컴파일 가능
- **UI**
  - 콘솔 기반, 라운드 테이블로 Client ID와 입찰금액 표시
  - 최고 입찰자와 경매 종료 알림 표시

---

## 컴파일 방법

```bash
gcc -o server server.c -lpthread
gcc -o client client.c

실행 방법

서버 실행

./server


경매 아이템 이름과 설명 입력

서버가 라운드 진행과 결과 출력

클라이언트 실행

./client


고유한 Client ID 입력

자유롭게 입찰 가능

입력 형식: 정수 단일값 (예: 100)

예시 사용
Server:
Enter auction item name: Antique Clock
Enter auction item description: Vintage wall clock from 1920s.

--- Round 1 start ---
Client ID | Bid
----------------
      101 | 0
      102 | 50
      103 | 30
----------------
Highest bidder this round: Client 102 with bid 50

--- Round 2 start ---
Client ID | Bid
----------------
      101 | 60
      102 | 50
      103 | 70
----------------
Highest bidder this round: Client 103 with bid 70

***** AUCTION OVER *****
Winner: Client 103 with bid 70

참고 사항

클라이언트는 서버의 라운드 시작 신호를 받지 않고 자유롭게 입찰 가능

서버는 라운드 종료 시 최종 테이블 출력

2라운드 이후 입찰값을 유지하여, 미입력 시 이전 값 그대로 적용

최대 10명의 클라이언트 지원 (MAX_CLIENTS 변경 가능)

개선 가능점

GUI 또는 ncurses 기반 인터페이스로 가시성 개선

라운드 타이머 및 클라이언트 측 카운트다운 표시

경매 기록 로그 파일 저장 기능 추가

네트워크 소켓을 통한 LAN 환경에서 원격 클라이언트 지원

작성자

이름: 마정훈

역할: 개발자

프로젝트: Unix Auction System

사용 기술: C 언어, POSIX 스레드, System V 메시지 큐, 동기화
