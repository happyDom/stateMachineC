#include <stdio.h>
#include "smDemo.h"

int main(int argc, char const *argv[])
{

    smDemoBuild();
    while (1)
    {
        printf("please input a key: ");
        inputKey = getchar();
        smDemoRun();
    }
    
    return 0;
}
