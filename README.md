시스템 프로그래밍 과제 - multi-processes & record lock

- 계좌 정보 파일에 여러 process가 동시에 접근하는 과정을 구현한다.

- 각 계좌정보는 다음과 같으며, 5개의 계좌에 대해 각 항목을 초기화한 파일이 제공된다. (파일명은 account.dat)
  • char acc_no[6]
  • char name[10]
  • int balance

- 10개의 child process를 생성하며, 각 child process는 operation.dat 파일에 지시된 명령을 수행한다.

- operation.dat는 20개의 명령으로 구성되며, 각 명령은 다음과 같다.
  • acc_no optype [amount]
  • optype은 w: 인출, d: 예치, i: 조회
  • 예:
  1111 w 10000 => acc_no 1111인 계좌에서 10000 인출
  2222 d 10000 => acc_no 2222인 계좌에 10000 예치(입금)
  3333 i => acc_no 3333인 계좌의 현재 잔액 출력
  7777 w 10000 => acc_no 7777이 없는 경우 “계좌정보 없음“ 출력

- 각 process는 매 명령을 수행한 후 다음과 같이 잠시 쉰다.
  • 이때,sleeptime은microsec단위로0~1000000usec사이의randomvalue
  를 발생시켜, 그 만큼 쉰다.

- 각 명령을 수행 한 후 화면에 그 결과를 출력해야 한다. (맨 앞에 자신의 process id를 표시해야 함)
  • 예:
  pid: 1234 acc_no: 1111 deposit: 10000 balance: 20000
  pid: 5768 acc_no: 2222 inquiry balance: 30000
  pid: 0053 acc_no: 3333 withdraw: 10000 balance: 10000

- 모든 child process가 종료된 후, main 함수(parent process)는 account.dat에 기록된 모든 계좌 내용을 화면에 출력한다.
  • 예:
  acc_no: 1111 balance: 50000
  .............
