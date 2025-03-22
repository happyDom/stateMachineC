#include <stdio.h>
#include "smDemo.h"

int main(int argc, char const *argv[])
{
    smDemoBuild();
    smDemoRun();    //运行一次状态机，进入初始状态

    inputKey = '0';
    while (1)
    {
        printf("please input a key: ");
        inputKey = getchar();
        if('\n' != inputKey){
            printf("%c is pressed\n", inputKey);
            getchar();   //读取空字符
        }
        smDemoRun();
    }
    
    return 0;
}
