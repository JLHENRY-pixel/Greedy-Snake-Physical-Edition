/*-----------------------------------------------------------------
2020.4.6更新：      加入45度角斜抛功能，优化墙角里碰撞反弹不了的bug
------------------------------------------------------------------
*/
#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <windows.h>
#define PI 3.14
#define V 26// 抛出速度  想改自己改
const double g = 10;
//////////全局变量
int width = 40, length = 150;
char map[1000][1000];
int i, j, old_x, old_y, flag, color, if_miss, step, max;
double x, y, Vx, Vy, s1, s2, X, Y;//Vx纵向，Vy横向
char wall = '#', air = ' ',brick='&';
double C[8] = { 0, 10, 30,45,70, 90 };
int flag = 1;
double x1_t1;
double x2_t1;
double x3_t1;
double x4_t1;
double s1, s2;
double temp;
double t1, t2;

///////
struct Step //轨迹结构体
{
    int x;
    int y;
} s[1000];
void gotoxy(int x, int y) //坐标函数  在windows.h中   不过用时要加上这一段（固定）
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(handle, pos);
}
void HideCursor() //光标隐藏
{
    CONSOLE_CURSOR_INFO cursor_info = { 1, 0 }; //后边的0代表光标不可见
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
void angle_switch()
{
    printf("-----选择初始抛出角度-----\n");
    gotoxy(length / 2, width + 4);
    printf("1.        0               \n");
    gotoxy(length / 2, width + 5);
    printf("2.       10               \n");
    gotoxy(length / 2, width + 6);
    printf("3.       30               \n");
    gotoxy(length / 2, width + 7);
    printf("4.       45               \n");
    gotoxy(length / 2, width + 8);
    printf("5.       70(推荐)         \n");
    gotoxy(length / 2, width + 9);
    printf("6.       90               \n");
    gotoxy(length / 2, width + 10);
    printf("-------按Q退出------------\n");
}


void init()
{
    HideCursor();//隐藏光标的函数
    gotoxy(0, 0);
    for (i = 0; i <= width; i++)
    { //生成地图模板并把坐标放入map中
        for (j = 0; j <= length; j++)
        {
            if (i == 0 || i == width || j == 0 || j == length  || (i == 2 * width / 3) && (j >= 63 && j <= 87))
                map[i][j] = wall;


            else
                map[i][j] = air;
        }
    }
    for (i = 0; i <= width; i++)
    { //布置砖块brick
        for (j = 0; j <= length; j++)
        {

            if (((i >= 20 && i <= 22) && (j >= 1 && j <= 50)) || ((i >= 20 && i <= 22) && (j >= 100 && j <= 149)))
                map[i][j] = brick;
            if ((i == 10 || i == 11) && ((j >= 1 && j <= 50) || (j >= 100 && j <= 149)))
                map[i][j] = brick;
        }
    }


    gotoxy(0, 0);
    //打印地图

    for (i = 0; i <= width; i++)
    {
        for (j = 0; j <= length; j++)
        {

            if (map[i][j] == wall)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 60);//控制wall's fore and background颜色
                printf("%c", wall);
            }
            if (map[i][j] == air)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);//air
                printf("%c", air);
            }
            if (map[i][j] == brick)
            {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 80);
                printf("%c", brick);
            }


        }
        printf("\n");//！！！防止乱码！！！
    }
    gotoxy(length / 2, width + 3);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
    angle_switch();
}




///////////抛物线
void sport()
{
    //蛇身颜色
    switch (color)
    {
    case 0:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 60);
        break; //蓝色
    case 1:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 70);
        break; //红色
    case 2:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 40);
        break; //绿色
    case 3:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 30);
        break; //深蓝色
    case 4:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 128);
        break; //紫色
    }
    HideCursor();
    gotoxy(s[step].y, s[step].x);
    printf(" ");
    map[s[step].x][s[step].y]++;//map此时作后台数组，节点坐标存入，防止自己吃自己
    step++;
    /*-------------------------------------------------------
    核心公式：   纵向：X = X0 + Vx*t + 1/2*VX*t*t;
                横向：Y = Y0 + Vy*t;

    核心思想：  如果选择时间作为移动参考，则每秒会出现点相差太远
                这里反过来，以连续为前提，尝试实现：

                分别算出 X 增加一个单位长度 应该增加的 Y 的距离 s1
                        Y 增加一个单位长度 应该增加的 X 的距离 s2
                        优先选择更短的作为下一次移动的基准。

                X Y 的增加都是1

                X,Y浮点型，两个方向的位移累加到他两身上，而gotoxy()坐标只能是int ，故每次用old_x,old_y来承接X,Y

                因为实体抛物线是连续的，每一段细微的位移理论上是连续的

                因此s1 和 s2 理论上只会是0和1以及少数出现2的跳节

                s1=0:下降较快         s2=0:抛物线顶端     都不为0：中间

     ------------------------------------------------------------*/
     //X Y是double变量增加精准度，但坐标就需要强制转换

     //当X增加一个单位方向时
     //5*t*t + Vx t - 1= 0;解一元二次方程（这里判断  增加后减少仍然是1    可能会有4种情况:Δ两解，还有向上向下的+/-1）
        ////////////X增加一个单位

    x1_t1 = (-Vx + sqrt(Vx * Vx + 20)) / 10;//下降
    x2_t1 = (-Vx - sqrt(Vx * Vx + 20)) / 10;//<0
    x3_t1 = (-Vx + sqrt(Vx * Vx - 20)) / 10;//上升
    x4_t1 = (-Vx - sqrt(Vx * Vx - 20)) / 10;//>0
    if (Vx >= 0) {
        t1 = x1_t1;
        s1 = t1 * Vy;
    }
    else {
        if (x3_t1 > 0 && x4_t1 > 0)
            temp = x3_t1 < x4_t1 ? x3_t1 : x4_t1;
        else
            temp = 10;
        s1 = temp * Vy;
    }


    //当Y增加一个单位方向时
    t2 = 1.0 / fabs(Vy);
    s2 = Vx * t2 + 0.5 * g * t2 * t2;
    //判断最优路线
    if ((int)s1 == 0)
        flag = 1;
    else if ((int)s2 == 0)
        flag = 2;
    else
        flag = s1 < s2 ? 1 : 2;//1,2跳节
    ////选择最优路线
    if (Vx >= 0) {
        if (flag == 1)
        {
            X = X + Vx * t1 + 5 * t1 * t1;
            Y = Y + s1;
            Vx = Vx + g * t1;
        }
        if (flag == 2)
        {
            Y = Y + Vy * t2;
            X = X + s2;
            Vx = Vx + g * t2;
        }
    }
    else
    {
        if (flag == 1)
        {
            X = X + Vx * temp + 5 * temp * temp;
            Y = Y + s1;
            Vx = Vx + g * temp;
        }
        if (flag == 2)
        {
            Y = Y + Vy * t2;
            X = X + s2;
            Vx = Vx + g * t2;
        }
    }

    //////////////
    //若有更好的算法，以上区域任意改//
   ///////////////
    old_x = (int)X;
    old_y = (int)Y;

    //常规的判断碰撞   加入墙角里的碰撞
    if (map[old_x + 1][old_y] == wall && Vx >= 0 && map[old_x][old_y + 1] != wall && map[old_x][old_y - 1] != wall)
    {    
            color = (color + 1) % 5;
            Vx = -Vx;
    }

    else if (map[old_x - 1][old_y] == wall && Vx <= 0 && map[old_x][old_y + 1] != wall && map[old_x][old_y - 1] != wall)
    {
        color = (color + 1) % 5;
        Vx = -Vx;
    }
    else if (map[old_x][old_y - 1] == wall && Vy <= 0 && map[old_x - 1][old_y] != wall && map[old_x + 1][old_y] != wall)
    {
        color = (color + 1) % 5;
        Vy = -Vy;
    }
    else if (map[old_x][old_y + 1] == wall && Vy >= 0 && map[old_x - 1][old_y] != wall && map[old_x + 1][old_y] != wall)
    {
        color = (color + 1) % 5;
        Vy = -Vy;
    }
    else if (map[old_x + 1][old_y + 1] == wall && Vx > 0 && Vy > 0 || map[old_x + 1][old_y - 1] == wall && Vx > 0 && Vy < 0 || map[old_x - 1][old_y + 1] == wall && Vx < 0 && Vy>0 || map[old_x - 1][old_y - 1] == wall && Vx < 0 && Vy < 0)
    {
        color = (color + 1) % 5;
        Vy = -Vy;
        Vx = -Vx;
    }
    if (map[old_x + 1][old_y] == brick && Vx >= 0)
    {
        //color = (color + 1) % 5;
        Vx = -Vx;
        map[old_x + 1][old_y] = air;
        gotoxy(old_y, old_x + 1);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
        printf(" ");
      
    }
    else if (map[old_x - 1][old_y] == brick && Vx <= 0)
    {
        //color = (color + 1) % 5;
        Vx = -Vx;
        map[old_x - 1][old_y] = air;
        gotoxy(old_y, old_x - 1);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
        printf(" ");
      
    }
    else if (map[old_x][old_y + 1] == brick && Vy >= 0)
    {
        //color = (color + 1) % 5;
        Vy = -Vy;
        map[old_x][old_y + 1] = air;
        gotoxy(old_y + 1, old_x);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
        printf(" ");
        
    }
    else if (map[old_x][old_y - 1] == brick && Vy <= 0)
    {
        //color = (color + 1) % 5;
        Vy = -Vy;
        map[old_x][old_y - 1] = air;
        gotoxy(old_y - 1, old_x);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
        printf(" ");
       
    }
    if (step >= max)
    {
        if_miss = 1; //如果长度大于最长长度就要开始从末尾消除尾节点了
    }
    step = step % max;

    ////限制长度,蛇的轨迹其实是带颜色的‘ ’，用黑色的‘ ’也按轨迹打印，即可“消去”尾巴
    if (if_miss == 1)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
        gotoxy(s[step].y, s[step].x);
        map[s[step].x][s[step].y]--;//轨迹点存入map数组能防止自己吃自己
        if (map[s[step].x][s[step].y] == air)
            printf("%c", air);
    }
    s[step].x = old_x;
    s[step].y = old_y;

    if (_kbhit())
    {
        if (getch() == 'q') {
            gotoxy(2, 2);
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
            printf("再见");
            exit(1);
        }
        else
            return;
    }
    Sleep(80 - 2 * old_x);//打点频率，与高度反相关
}

int main()
{
    char ch;
    int angle = 0;
    if_miss = 0;
    step = 0;
    color = 0;
    max = 80;//蛇身长度

    //开始位置
    X = width - 4;
    Y = 4;
    old_x = (int)x;//坐标只能整数
    old_y = (int)y;

    s[step].x = old_x;
    s[step].y = old_y;
    system("mode con cols=180 lines=60");
    system("color 07");
    init();

    ch = getch();
    if (ch <= '6') {
        angle = (int)(ch - '1');
        gotoxy(length / 2, width + 11);
        printf("%c,您输入的是%d度", ch, (int)C[angle]);
    }
    else
    {
        HideCursor();
        gotoxy(length / 2, width + 11);
        printf("输入错误");
    }
    // a=θ*pi/180
    Vx = -V * sin(C[angle] * PI / 180.0); //速度向右向下为正方向 
    Vy = V * cos(C[angle] * PI / 180.0);
    while (1)
    {
        sport();
    }
}