// liitle_fs_store.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "little_fs_time_stamp_store.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    srand((unsigned)time(NULL));
    char name[6] = {"yuhao"};
    char buff[100] = {0};
    int count = rand() % 100;

    memset(buff, 0, 100);
    uint32_t time_stamp[100] = {0};
    for (int j = 0; j < 10; j++)
    {
        for (int i = 0; i < count; i++)
        {
            *(buff + i) = rand() % 12 + 65;
        }
        time_stamp[j] = rand() % 100;
    }
    little_fs_time_stamp_store_save(name, time_stamp[0], buff, count);
    little_fs_time_stamp_store_read(name, time_stamp[0], buff, count);
    little_fs_time_stamp_store_load(name, time_stamp[0], buff, count);
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5.
//   转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
