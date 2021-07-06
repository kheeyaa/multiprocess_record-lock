// 전자공학과 201800384 강희 정상동작
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

struct account
{
    char acc_no[6]; // account number
    char name[10];  // account owner
    int balance;    // balance
};
struct operation
{
    char acc_no[6];
    char optype[20];
    int amount;
};

int reclock(int fd, int recno, int len, int type)
{
    struct flock fl;
    switch (type)
    {
    case F_RDLCK:
    case F_WRLCK:
    case F_UNLCK:
        fl.l_type = type;
        fl.l_whence = SEEK_SET;
        fl.l_start = recno * len;
        fl.l_len = len;
        fcntl(fd, F_SETLKW, &fl);
        //	printf("fcntl %d\n",(fcntl (fd, F_SETLKW, &fl)));
        return 1;
    default:
        return -1;
    };
}
int process_func()
{
    int fd_acc, fd2;
    FILE *fp_acc;
    struct account current;

    int accFind = 0;
    int accFindNum = 0;
    int op_num = 0;
    int pos, n;

    FILE *fp_op;
    char command[20][100] = {
        0,
    };
    char com_amount[20][20] = {
        0,
    };
    struct operation com[20];

    // 1. 명령어 파일 읽기
    fp_op = fopen("./operation.dat", "r");

    for (int i = 0; i < 20; i++)
    {
        fgets(command[i], 100, fp_op);

        int cnt = 0;
        int k = 0;
        for (int j = 0; j < 100; j++)
        {
            if (command[i][j] == '\n')
                break;
            if ((command[i][j] >= '0' && command[i][j] <= '9') ||
                (command[i][j] >= 'a' && command[i][j] <= 'z'))
            {
                if (cnt == 0)
                {
                    com[i].acc_no[k] = command[i][j];
                    k++;
                }
                else if (cnt == 1)
                {
                    com[i].optype[k] = command[i][j];
                    k++;
                }
                else if (cnt == 2)
                {
                    com_amount[i][k] = command[i][j];
                    k++;
                }
            }
            else if (cnt == 0 && k != 0)
            {
                cnt = 1;
                com[i].acc_no[k] = '\0';
                k = 0;
            }
            else if (cnt == 1 && k != 0)
            {
                cnt = 2;
                com[i].optype[k] = '\0';
                k = 0;
            }
            else if (cnt == 2 & k != 0)
            {
                com_amount[i][k] = '\0';
                k = 0;
            }
        }
    }
    // 2. amount값 문자열에서 정수로 바꾸기
    for (int i = 0; i < 20; i++)
    {
        com[i].amount = atoi(com_amount[i]);
    }

    fclose(fp_op);

    // 3. 계좌 처리
    for (int i = 0; i < 20; i++)
    {
        accFind = accFindNum = op_num = 0;

        fd_acc = open("./account.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        fp_acc = fdopen(fd_acc, "r");

        // 3-1. 계좌 정보 확인
        while (fread(&current, sizeof(struct account), 1, fp_acc))
        {
            if (strcmp(current.acc_no, com[i].acc_no) == 0)
            {
                // 3-1-1 계좌 정보 존재
                accFind = 1;
                break;
            }
            else
                accFindNum += 1; //계좌 정보 위치
        }
        if (accFind == 0)
        { // 3-1-2 계좌 존재하지 않음
            printf("pid: %ld\t계좌정보 없음\n", (long)getpid());
            continue;
        }

        if (com[i].optype[0] == 'w')
        { // 3-2 withdraw
            rewind(fp_acc);
            reclock(fd_acc, accFindNum, sizeof(struct account), F_WRLCK);
            pos = accFindNum * sizeof(current);
            lseek(fd_acc, (long)pos, SEEK_SET);
            n = read(fd_acc, &current, sizeof(struct account));

            current.balance -= com[i].amount;
            rewind(fp_acc);
            lseek(fd_acc, (long)pos, SEEK_SET);
            write(fd_acc, &current, sizeof(struct account));

            printf("pid: %ld acc_no:%s\twithdraw:%d\tbalance:%d\n", (long)getpid(), com[i].acc_no, com[i].amount, current.balance);
            reclock(fd_acc, accFindNum, sizeof(struct account), F_UNLCK);
        }
        else if (com[i].optype[0] == 'i')
        { // 3-3 inquiry
            reclock(fd_acc, accFindNum, sizeof(struct account), F_RDLCK);
            pos = accFindNum * sizeof(current);
            rewind(fp_acc);
            lseek(fd_acc, (long)pos, SEEK_SET);
            n = read(fd_acc, &current, sizeof(struct account));

            printf("pid: %ld acc_no:%s\tinquiry\tbalance:%d\n", (long)getpid(), com[i].acc_no, current.balance);
            reclock(fd_acc, accFindNum, sizeof(struct account), F_UNLCK);
        }
        else if (com[i].optype[0] == 'd')
        { // 3-4 deposit
            rewind(fp_acc);
            reclock(fd_acc, accFindNum, sizeof(struct account), F_WRLCK);
            pos = accFindNum * sizeof(current);
            lseek(fd_acc, (long)pos, SEEK_SET);
            n = read(fd_acc, &current, sizeof(struct account));

            current.balance += com[i].amount;
            rewind(fp_acc);
            lseek(fd_acc, (long)pos, SEEK_SET);
            write(fd_acc, &current, sizeof(struct account));

            printf("pid: %ld acc_no:%s\tdeposit:%d\tbalance:%d\n", (long)getpid(), com[i].acc_no, com[i].amount, current.balance);
            reclock(fd_acc, accFindNum, sizeof(struct account), F_UNLCK);
        }
        // 5. sleep
        int random;
        srand((unsigned)time(NULL));
        random = rand() % 1000000;
        usleep(random);
    }

    fclose(fp_acc);
    close(fd_acc);
    exit(0);
}

int main()
{

    int fd_acc;
    FILE *fp_acc;
    struct account current;

    pid_t pids[10];
    int i;
    int n = 10;

    // 1. 10 개 자식 프로세스 생성
    for (i = 0; i < n; ++i)
    {
        if ((pids[i] = fork()) < 0)
        {
            perror("fork");
            abort();
        }
        else if (pids[i] == 0)
        {
            process_func();
            exit(0);
        }
    }

    // 2. 10개 자식 프로세스 기다림
    int status;
    pid_t pid;
    while (n > 0)
    {
        pid = wait(&status);
        --n; // TODO(pts): Remove pid from the pids array.
    }

    // 3. 결과 출력
    printf("------------------------------------------------------------\n");

    fd_acc = open("./account.dat", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    fp_acc = fdopen(fd_acc, "r");
    lseek(fd_acc, 0, SEEK_SET);

    while (fread(&current, sizeof(struct account), 1, fp_acc))
    {
        printf("acc_no:%s\tbalance:%d\n", current.acc_no, current.balance);
    }

    close(fd_acc);
    fclose(fp_acc);

    return 0;
}
