#include <stdio.h>
#include "smDemo.h"

int main(int argc, char const *argv[])
{
    smDemoBuild();
    while (1)
    {
        printf("please input a key: ");
        inputKey = getchar();
        if(inputKey >= 'a' && inputKey <='z'){
            smDemoRun();
        }
    }
    
    return 0;
}
