#include<stdio.h>
#include "smDemo.h"

int main(void){
    //创建状态机
    smDemoBuild(0);

    while (1)
    {
        printf("please press a key and then Enter it: ");
        inputKey = getchar();
        if('a' <= inputKey && inputKey <= 'z')
        {
            smDemoRun();
        }
    }
}