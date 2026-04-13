#include<stdio.h>
#include<time.h>
#include<windows.h>
#include<stdlib.h>
#include<string.h>
#include<conio.h>

#define U 1
#define D 2
#define L 3
#define R 4       // 蛇的状态：U:上 D:下 L:左 R:右

// 用户相关宏定义
#define MAX_USERNAME 32
#define MAX_PASSWORD 32
#define USER_FILE    "users.dat"
#define LOG_FILE     "gamelog.dat"

typedef struct SNAKE // 蛇的每一个节点
{
    int x;
    int y;
    struct SNAKE* next;
}snake;

// 全局变量 //
int score = 0, add = 10;        // 总得分，每次吃食物得分
int status, sleeptime = 200;    // 每次运行的时间间隔
snake* head, * food;            // 蛇头指针，食物指针
snake* q;                       // 遍历蛇体时用到的指针
int endgamestatus = 0;          // 游戏结束状态：1撞墙 2咬到自己 3主动退出游戏
int afterGameChoice = 0;        // 游戏结束后的选择：1=再来一把 0=回到主界面
char currentUser[MAX_USERNAME] = ""; // 当前登录用户名
int  currentUserID = 0;              // 当前登录用户ID
time_t gameStartTime = 0;            // 本局游戏开始时间

// 声明全部函数 //
void Pos(int x, int y);
void creatMap();
void initsnake();
int  biteself();
void createfood();
void cantcrosswall();
void snakemove();
void pause();
void gamecircle();
void welcometogame();
void endgame();
void gamestart();
// 新增功能函数声明
int  userExists(const char* username);
int  registerUser();
int  loginUser();
void saveGameLog(int endStatus, int finalScore);
void showGameLog();
void readPassword(char* buf, int maxLen);
void showMainMenu();

void Pos(int x, int y) // 设置光标位置
{
    COORD pos;
    HANDLE hOutput;
    pos.X = x;
    pos.Y = y;
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOutput, pos);
}

void creatMap() // 创建地图
{
    int i;
    for (i = 0; i < 58; i += 2) // 打印上下边框
    {
        Pos(i, 0);
        printf("■");
        Pos(i, 26);
        printf("■");
    }
    for (i = 1; i < 26; i++) // 打印左右边框
    {
        Pos(0, i);
        printf("■");
        Pos(56, i);
        printf("■");
    }
}

void initsnake() // 初始化蛇体
{
    snake* tail;
    int i;
    tail = (snake*)malloc(sizeof(snake));
    tail->x = 24;
    tail->y = 5;
    tail->next = NULL;
    for (i = 1; i <= 4; i++)
    {
        head = (snake*)malloc(sizeof(snake));
        head->next = tail;
        head->x = 24 + 2 * i;
        head->y = 5;
        tail = head;
    }
    while (tail != NULL)
    {
        Pos(tail->x, tail->y);
        printf("■");
        tail = tail->next;
    }
}

int biteself() // 判断是否咬到了自己
{
    snake* self;
    self = head->next;
    while (self != NULL)
    {
        if (self->x == head->x && self->y == head->y)
            return 1;
        self = self->next;
    }
    return 0;
}

void createfood() // 随机创建食物
{
    snake* food_1;
    food_1 = (snake*)malloc(sizeof(snake));
    // 确保x坐标为偶数（2,4,...,52），y在边界内
    do {
        food_1->x = (rand() % 26 + 1) * 2;
        food_1->y = rand() % 24 + 1;
        // 检查是否与蛇体重合
        int overlap = 0;
        q = head;
        while (q != NULL)
        {
            if (q->x == food_1->x && q->y == food_1->y)
            {
                overlap = 1;
                break;
            }
            q = q->next;
        }
        if (!overlap) break;
    } while (1);
    Pos(food_1->x, food_1->y);
    food = food_1;
    printf("$");
}

void cantcrosswall() // 不能穿墙
{
    if (head->x == 0 || head->x == 56 || head->y == 0 || head->y == 26)
    {
        endgamestatus = 1;
        endgame();
    }
}

void snakemove() // 蛇前进：上U 下D 左L 右R
{
    snake* nexthead;
    cantcrosswall();
    if (endgamestatus) return; // 撞墙后直接返回，不继续移动

    nexthead = (snake*)malloc(sizeof(snake));
    if (status == U)
    {
        nexthead->x = head->x;
        nexthead->y = head->y - 1;
        if (nexthead->x == food->x && nexthead->y == food->y)
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            score = score + add;
            createfood();
        }
        else
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == D)
    {
        nexthead->x = head->x;
        nexthead->y = head->y + 1;
        if (nexthead->x == food->x && nexthead->y == food->y)
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            score = score + add;
            createfood();
        }
        else
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == L)
    {
        nexthead->x = head->x - 2;
        nexthead->y = head->y;
        if (nexthead->x == food->x && nexthead->y == food->y)
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            score = score + add;
            createfood();
        }
        else
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == R)
    {
        nexthead->x = head->x + 2;
        nexthead->y = head->y;
        if (nexthead->x == food->x && nexthead->y == food->y)
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            score = score + add;
            createfood();
        }
        else
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (biteself() == 1)
    {
        endgamestatus = 2;
        endgame();
        return; // 咬到自己后直接返回
    }
}

void pause() // 暂停
{
    while (1)
    {
        Sleep(300);
        if (GetAsyncKeyState(VK_SPACE))
            break;
    }
}

void gamecircle() // 控制游戏
{
    Pos(64, 10);
    printf("得分：%d  ", score);
    Pos(64, 11);
    printf("每次食物得分：%d分", add);
    Pos(64, 13);
    printf("不能穿墙，不能咬到自己");
    Pos(64, 14);
    printf("↑.↓.←.→分别控制蛇的移动.");
    Pos(64, 15);
    printf("F1 为加速，F2 为减速");
    Pos(64, 16);
    printf("ESC :退出游戏. space:暂停游戏.");
    Pos(64, 17);
    printf("按F5显示游戏用户日志");
    Pos(64, 18);
    printf("*** %s 正在游戏中 ***", currentUser);

    status = R;
    while (1)
    {
        Pos(64, 10);
        printf("得分：%d  ", score);
        Pos(64, 11);
        printf("每次食物得分：%d分", add);

        if (GetAsyncKeyState(VK_UP) && status != D)
            status = U;
        else if (GetAsyncKeyState(VK_DOWN) && status != U)
            status = D;
        else if (GetAsyncKeyState(VK_LEFT) && status != R)
            status = L;
        else if (GetAsyncKeyState(VK_RIGHT) && status != L)
            status = R;
        else if (GetAsyncKeyState(VK_SPACE))
            pause();
        else if (GetAsyncKeyState(VK_ESCAPE))
        {
            endgamestatus = 3;
            endgame(); // ESC直接在这里处理结束
            break;
        }
        else if (GetAsyncKeyState(VK_F1))
        {
            if (sleeptime >= 50)
            {
                sleeptime -= 30;
                add += 2;
                if (sleeptime == 320) add = 2;
            }
        }
        else if (GetAsyncKeyState(VK_F2))
        {
            if (sleeptime < 350)
            {
                sleeptime += 30;
                add -= 2;
                if (sleeptime == 350) add = 1;
            }
        }
        else if (GetAsyncKeyState(VK_F5))
        {
            showGameLog();
            // 恢复游戏界面
            system("cls");
            creatMap();
            snake* tmp = head;
            while (tmp != NULL) { Pos(tmp->x, tmp->y); printf("■"); tmp = tmp->next; }
            Pos(food->x, food->y); printf("★");
            Pos(64, 10); printf("得分：%d  ", score);
            Pos(64, 11); printf("每次食物得分：%d分", add);
            Pos(64, 13); printf("不能穿墙，不能和到自己");
            Pos(64, 14); printf("↑.↓.←.→分别控制蛇的移动.");
            Pos(64, 15); printf("F1 为加速，F2 为减速");
            Pos(64, 16); printf("ESC :退出游戏. space:暂停游戏.");
            Pos(64, 17); printf("按F5显示游戏用户日志");
            Pos(64, 18); printf("*** %s 正在游戏中 ***", currentUser);
        }
        Sleep(sleeptime);
        snakemove();
        if (endgamestatus) break; // 死亡时退出循环（endgame已在snakemove内调用）
    }
}

void welcometogame() // 开始界面
{
    Pos(40, 12);
    printf("欢迎来到贪吃蛇游戏！");
    Pos(40, 25);
    system("pause");
    system("cls");
    Pos(25, 12);
    printf("↑.↓.←.→分别控制蛇的移动  F1 为加速，F2 为减速");
    Pos(25, 13);
    printf("加速将获得更高的分数！");
    system("pause");
    system("cls");
}

void endgame() // 结束游戏
{
    saveGameLog(endgamestatus, score);

    // 释放蛇和食物内存
    snake* tmp;
    while (head != NULL) { tmp = head; head = head->next; free(tmp); }
    if (food != NULL) { free(food); food = NULL; }
    head = NULL; q = NULL;

    while (1)
    {
        system("cls");
        Pos(20, 9);
        if (endgamestatus == 1)
            printf("对不起，您撞到墙了！游戏结束！");
        else if (endgamestatus == 2)
            printf("对不起，您咬到自己了！游戏结束！");
        else if (endgamestatus == 3)
            printf("您已经退出了游戏。");
        Pos(20, 10);
        printf("本局得分：%d 分", score);
        Pos(20, 12);
        printf("1. 再来一把");
        Pos(20, 13);
        printf("2. 查看日志");
        Pos(20, 14);
        printf("3. 回到主界面");
        Pos(20, 16);
        printf("请选择（1-3）：");

        FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
        int ch = 0;
        scanf("%d", &ch);
        while (getchar() != '\n');

        if (ch == 1)
        {
            // 重置游戏变量，调用方循环重新调用 gamestart()
            score = 0; add = 10; sleeptime = 200; endgamestatus = 0;
            afterGameChoice = 1;
            return;
        }
        else if (ch == 2)
        {
            showGameLog();
            // showGameLog 结束后继续循环，重新显示结束菜单
        }
        else if (ch == 3)
        {
            score = 0; add = 10; sleeptime = 200; endgamestatus = 0;
            afterGameChoice = 0;
            return;
        }
        // 无效输入：继续循环重新显示菜单
    }
}

// 保存游戏日志
void saveGameLog(int endStatus, int finalScore)
{
    FILE* fp = fopen(LOG_FILE, "a");
    if (fp == NULL) return;

    // 开始时间
    struct tm* st = localtime(&gameStartTime);
    char startStr[32];
    strftime(startStr, sizeof(startStr), "%Y-%m-%d %H:%M:%S", st);

    // 持续时长（秒）
    time_t endTime = time(NULL);
    long duration = (long)(endTime - gameStartTime);
    int dur_min = (int)(duration / 60);
    int dur_sec = (int)(duration % 60);

    fprintf(fp, "%d|%s|%s|%02d:%02d|%d\n",
            currentUserID, currentUser, startStr, dur_min, dur_sec, finalScore);
    fclose(fp);
}

// 显示游戏日志（按F5调用）
void showGameLog()
{
    system("cls");
    printf("\n  ========== 游戏用户日志 ==========\n\n");
    printf("  %-6s  %-16s  %-21s  %-8s  %s\n",
           "ID", "用户名", "开始时间", "时长", "得分");
    printf("  %-6s  %-16s  %-21s  %-8s  %s\n",
           "------", "----------------", "---------------------", "--------", "------");

    FILE* fp = fopen(LOG_FILE, "r");
    if (fp == NULL)
    {
        printf("  暂无游戏记录。\n");
    }
    else
    {
        int uid, pts;
        char uname[MAX_USERNAME], startStr[32], dur[16];
        int count = 0;
        while (fscanf(fp, "%d|%31[^|]|%31[^|]|%8[^|]|%d\n",
                      &uid, uname, startStr, dur, &pts) == 5)
        {
            printf("  %-6d  %-16s  %-21s  %-8s  %d\n",
                   uid, uname, startStr, dur, pts);
            count++;
        }
        fclose(fp);
        if (count == 0)
            printf("  暂无游戏记录。\n");
    }

    printf("\n  按任意键返回游戏...\n");
    system("pause>nul");
}

void gamestart() // 游戏开始前
{
    system("mode con cols=100 lines=30");
    // 刷新输入缓冲，防止主菜单残留按键触发游戏内pause()
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    creatMap();
    initsnake();
    createfood();
    gameStartTime = time(NULL); // 记录游戏开始时间
}

// 隐式读入密码，将输入显示为 *
void readPassword(char* buf, int maxLen)
{
    int i = 0;
    int ch;
    while (i < maxLen - 1)
    {
        ch = _getch();
        if (ch == '\r' || ch == '\n') break; // 回车结束
        if (ch == '\b' || ch == 127)          // 退格
        {
            if (i > 0)
            {
                i--;
                printf("\b \b"); // 擦除屏幕上的 *
            }
            continue;
        }
        if (ch >= 32 && ch <= 126) // 可打印字符
        {
            buf[i++] = (char)ch;
            printf("*");
        }
    }
    buf[i] = '\0';
    printf("\n");
}

// 检查用户名是否已存在（返回1存在，0不存在）
int userExists(const char* username)
{
    FILE* fp = fopen(USER_FILE, "r");
    if (fp == NULL) return 0;
    char uname[MAX_USERNAME], pwd[MAX_PASSWORD];
    int id;
    while (fscanf(fp, "%d %31s %31s", &id, uname, pwd) == 3)
    {
        if (strcmp(uname, username) == 0)
        {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// 用户注册（返回1成功，0失败）
int registerUser()
{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char confirm[MAX_PASSWORD];

    system("cls");
    printf("\n\n");
    printf("  ========== 用户注册 ==========\n\n");

    while (1)
    {
        printf("  请输入用户名（不超过31个字符）: ");
        scanf("%31s", username);
        if (userExists(username))
            printf("  该用户名已存在，请换一个。\n");
        else
            break;
    }

    while (1)
    {
        printf("  请输入密码（不超过31个字符）: ");
        readPassword(password, MAX_PASSWORD);
        printf("  请再次确认密码: ");
        readPassword(confirm, MAX_PASSWORD);
        if (strcmp(password, confirm) == 0)
            break;
        printf("  两次密码不一致，请重新输入。\n");
    }

    FILE* fp = fopen(USER_FILE, "a");
    if (fp == NULL)
    {
        printf("  注册失败：无法写入用户文件。\n");
        return 0;
    }
    // 生成新用户ID = 当前文件中的用户数 + 1
    int newId = 1;
    FILE* fr = fopen(USER_FILE, "r");
    if (fr != NULL)
    {
        char u[MAX_USERNAME], p[MAX_PASSWORD];
        int id;
        int cnt = 0;
        while (fscanf(fr, "%d %31s %31s", &id, u, p) == 3) cnt++;
        fclose(fr);
        newId = cnt + 1;
    }
    fprintf(fp, "%d %s %s\n", newId, username, password);
    fclose(fp);

    currentUserID = newId;
    printf("\n  注册成功！用户名：%s  ID：%d\n", username, newId);
    strncpy(currentUser, username, MAX_USERNAME - 1);
    return 1;
}

// 用户登录（返回1成功，0失败）
int loginUser()
{
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int attempts = 0;
    const int MAX_ATTEMPTS = 3;

    system("cls");
    printf("\n\n");
    printf("  ========== 用户登录 ==========\n\n");

    while (attempts < MAX_ATTEMPTS)
    {
        printf("  请输入用户名: ");
        scanf("%31s", username);
        printf("  请输入密码: ");
        readPassword(password, MAX_PASSWORD);

        FILE* fp = fopen(USER_FILE, "r");
        if (fp == NULL)
        {
            printf("  用户文件不存在，请先注册。\n");
            return 0;
        }
        char uname[MAX_USERNAME], pwd[MAX_PASSWORD];
        int uid = 0;
        int found = 0;
        while (fscanf(fp, "%d %31s %31s", &uid, uname, pwd) == 3)
        {
            if (strcmp(uname, username) == 0 && strcmp(pwd, password) == 0)
            {
                found = 1;
                break;
            }
        }
        fclose(fp);

        if (found)
        {
            currentUserID = uid;
            strncpy(currentUser, username, MAX_USERNAME - 1);
            printf("\n  登录成功！欢迎，%s（ID:%d）！\n", username, uid);
            Sleep(1000);
            return 1;
        }
        else
        {
            attempts++;
            if (attempts < MAX_ATTEMPTS)
                printf("  用户名或密码错误，还有 %d 次机会。\n\n", MAX_ATTEMPTS - attempts);
        }
    }
    printf("  登录失败次数过多，请重试。\n");
    Sleep(1200);
    return 0;
}

// 主菜单
void showMainMenu()
{
    system("cls");
    printf("\n\n");
    printf("  ==========================================\n");
    printf("        欢迎来到 贪吃蛇游戏\n");
    printf("  ==========================================\n\n");
    if (currentUser[0] != '\0')
        printf("  当前登录：%s (ID:%d)\n\n", currentUser, currentUserID);
    else
        printf("  显示状态：未登录\n\n");
    printf("  1. 注册账户\n");
    printf("  2. 登录\n");
    printf("  3. 进入游戏\n");
    printf("  4. 查看游戏日志 (F5)\n");
    printf("  5. 退出\n\n");
    printf("  请选择 [1-5]: ");
}

int main()
{
    // 设置控制台为UTF-8编码，解决中文乱码
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    // 初始化随机数种子（全局只需一次）
    srand((unsigned)time(NULL));

    system("mode con cols=100 lines=30");

    while (1)
    {
        showMainMenu();

        int choice = 0;
        scanf("%d", &choice);
        // 清除输入缓冲区
        int c; while ((c = getchar()) != '\n' && c != EOF) {}

        switch (choice)
        {
        case 1: // 注册
            if (!registerUser())
                printf("\n  注册失败，请重试。\n");
            system("pause");
            break;

        case 2: // 登录
            loginUser();
            system("pause");
            break;

        case 3: // 进入游戏
            if (currentUser[0] == '\0')
            {
                printf("\n  请先登录再进入游戏！\n");
                system("pause");
                break;
            }
            do {
                afterGameChoice = 0;
                gamestart();
                gamecircle();
            } while (afterGameChoice == 1);
            break;

        case 4: // 查看日志
        {
            // 展示日志页（不需要返回游戏，直接返回菜单）
            system("cls");
            printf("\n  ========== 游戏用户日志 ==========\n\n");
            printf("  %-6s  %-16s  %-21s  %-8s  %s\n",
                   "ID", "用户名", "开始时间", "时长", "得分");
            printf("  %-6s  %-16s  %-21s  %-8s  %s\n",
                   "------", "----------------", "---------------------",
                   "--------", "------");
            FILE* fp = fopen(LOG_FILE, "r");
            if (fp == NULL)
            {
                printf("  暂无游戏记录。\n");
            }
            else
            {
                int uid, pts;
                char uname[MAX_USERNAME], startStr[32], dur[16];
                int cnt = 0;
                while (fscanf(fp, "%d|%31[^|]|%31[^|]|%8[^|]|%d\n",
                              &uid, uname, startStr, dur, &pts) == 5)
                {
                    printf("  %-6d  %-16s  %-21s  %-8s  %d\n",
                           uid, uname, startStr, dur, pts);
                    cnt++;
                }
                fclose(fp);
                if (cnt == 0) printf("  暂无游戏记录。\n");
            }
            printf("\n  按任意键返回菜单...\n");
            system("pause>nul");
            break;
        }

        case 5: // 退出
            system("cls");
            printf("\n  感谢使用，再见！\n\n");
            return 0;

        default:
            printf("  无效选项，请输入 1-5。\n");
            Sleep(800);
            break;
        }
    }
    return 0;
}
