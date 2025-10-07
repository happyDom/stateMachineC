#include <stdio.h>
#include "smDemo.h"

char keys[7] = {'a','b','c','d','e','f','g'};

char inputKey;
stateMachine_t demoSM;

void actionEntry(smUnit_t *pSt)
{
    if(pSt->pSm->roundCounter == 0){
        //状态机首次运行时
        printf("the current state is: %c\n", keys[pSt->stateID]);
    }else{
        //状态机切换进入某一状态时
        printf(" --> %c\n", keys[pSt->stateID]);
    }
}

void actionDo(smUnit_t *pSt)
{
    //状态机的do事件
    printf("----roundCounter of %c is %d\n", keys[pSt->stateID], pSt->roundCounter);
}

void actionExit(smUnit_t *pSt)
{
    //状态机的退出事件
    printf("state exchanged: %c", keys[pSt->stateID]);
}

smEventResult_t pressA(smUnit_t *pSt) {return 'a' == inputKey;};
smEventResult_t pressB(smUnit_t *pSt) {return 'b' == inputKey;};
smEventResult_t pressC(smUnit_t *pSt) {return 'c' == inputKey;};
smEventResult_t pressD(smUnit_t *pSt) {return 'd' == inputKey;};

void smDemoBuild()
{
    fsm_init(&demoSM, stateID_count, 0);
    
    // 注册状态动作
    // 为每一个状态指定进入事件，逗留事件和退出事件
    // 如果你的某些状态不需要全部的事件，则将不需要对应事件的位置使用NULL做入参即可
    fsm_actionSignUp(&demoSM, a, actionEntry, actionDo, actionExit);
    fsm_actionSignUp(&demoSM, b, actionEntry, actionDo, actionExit);
    fsm_actionSignUp(&demoSM, c, actionEntry, actionDo, actionExit);
    fsm_actionSignUp(&demoSM, d, actionEntry, actionDo, actionExit);
    
    // 注册状态事件
    // 为每个状态跳转路径，指定对应的事件
    fsm_eventSignUp(&demoSM, a, b, pressB);
    fsm_eventSignUp(&demoSM, a, c, pressC);
    fsm_eventSignUp(&demoSM, b, d, pressD);
    fsm_eventSignUp(&demoSM, c, a, pressA);
    fsm_eventSignUp(&demoSM, d, a, pressA);
    fsm_eventSignUp(&demoSM, d, b, pressB);

    //状态机配置完成后，打印状态机使用的内存块的数量
    printf("the blockSize needed is: %d\n", dyMM_blocksNumOfUsed());
}

/*
包装一下 fsm_run 函数，以便在引用时更直观
*/
void smDemoRun(void)
{
    fsm_run(&demoSM);
}