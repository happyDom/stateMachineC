#include <stdio.h>
#include "smDemo.h"

char keys[7] = {'a','b','c','d','e','f','g'};

char inputKey;
stateMachine_t demoSM;

void actionEntry(stateMachineUnit_t *pSt)
{
    if(pSt->pSm->roundCounter == 0){
        //状态机首次运行时
        printf("the current state is: %c\n", keys[pSt->stateID]);
    }else{
        //状态机切换进入某一状态时
        printf(" --> %c\n", keys[pSt->stateID]);
    }
}

void actionDo(stateMachineUnit_t *pSt)
{
    //状态机的do事件
    printf("----roundCounter of %c is %d\n", keys[pSt->stateID], pSt->roundCounter);
}

void actionExit(stateMachineUnit_t *pSt)
{
    //状态机的退出事件
    printf("state exchanged: %c", keys[pSt->stateID]);
}

stateMachine_eventResult_t pressA(stateMachineUnit_t *pSt) {return 'a' == inputKey;};
stateMachine_eventResult_t pressB(stateMachineUnit_t *pSt) {return 'b' == inputKey;};
stateMachine_eventResult_t pressC(stateMachineUnit_t *pSt) {return 'c' == inputKey;};
stateMachine_eventResult_t pressD(stateMachineUnit_t *pSt) {return 'd' == inputKey;};

void smDemoBuild()
{
    fsm_init(&demoSM, stateID_count, 0);
    
    // 注册状态动作
    // 为每一个状态指定进入事件，逗留事件和退出事件
    // 如果你的某些状态不需要全部的事件，则将不需要对应事件的位置使用NULL做入参即可
    demoSM.actionSignUp(&demoSM, a, actionEntry, actionDo, actionExit);
    demoSM.actionSignUp(&demoSM, b, actionEntry, actionDo, actionExit);
    demoSM.actionSignUp(&demoSM, c, actionEntry, actionDo, actionExit);
    demoSM.actionSignUp(&demoSM, d, actionEntry, actionDo, actionExit);
    
    // 注册状态事件
    // 为每个状态跳转路径，指定对应的事件
    demoSM.eventSingUp(&demoSM, a, b, pressB);
    demoSM.eventSingUp(&demoSM, a, c, pressC);
    demoSM.eventSingUp(&demoSM, b, d, pressD);
    demoSM.eventSingUp(&demoSM, c, a, pressA);
    demoSM.eventSingUp(&demoSM, d, a, pressA);
    demoSM.eventSingUp(&demoSM, d, b, pressB);

    //状态机配置完成后，打印状态机使用的内存块的数量
    printf("the blockSize needed is: %d\n", dyMM_blocksNumOfUsed());
}

/*
包装一下 fsm_run 函数，以便在引用时更直观
*/
void smDemoRun(void)
{
    demoSM.run(&demoSM);
}