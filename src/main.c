#include "smDemo.h"

int main(void){
    //创建状态机
    smDemoBuild();

    while (1)
    {
        printf("please press a key and then Enter it: ");
        inputKey = getchar();
        if('a' <= inputKey && inputKey <= 'z')
        {
            //运行状态机
            smDemoRun();
        }
    }
}