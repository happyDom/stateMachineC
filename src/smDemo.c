#include <stdio.h>
#include "smDemo.h"

char keys[7] = {'a','b','c','d','e','f','g'};

char inputKey;
stateMachine_t demoSM;
stateMachine_t abcSM;

void actionEntry(stateMachineUnit_t *pSt)
{
    printf("%c is pressed, state enter  to: %c\n", inputKey, keys[pSt->stateID]);
}

void actionDo(stateMachineUnit_t *pSt)
{
    printf("roundCounter of %c is %d\n", keys[pSt->stateID], pSt->roundCounter);
}

void actionExit(stateMachineUnit_t *pSt)
{
    printf("%c is pressed, state exist from: %c\n", inputKey, keys[pSt->stateID]);
}

stateMachine_eventResult_t pressA(stateMachineUnit_t *pSt) {return 'a' == inputKey;};
stateMachine_eventResult_t pressB(stateMachineUnit_t *pSt) {return 'b' == inputKey;};
stateMachine_eventResult_t pressC(stateMachineUnit_t *pSt) {return 'c' == inputKey;};
stateMachine_eventResult_t pressD(stateMachineUnit_t *pSt) {return 'd' == inputKey;};

void smDemoBuild()
{
    fsm_init(&demoSM, stateID_end, 0);
    fsm_init(&abcSM,2,0);
    
    // 注册状态动作
    fsm_actionSignUp(&demoSM, a, actionEntry, actionDo, actionExit);
    fsm_actionSignUp(&demoSM, b,actionEntry, actionDo, actionExit);
    fsm_actionSignUp(&demoSM, c,actionEntry, actionDo, actionExit);
    fsm_actionSignUp(&demoSM, d,actionEntry, actionDo, actionExit);
    
    // 注册状态事件
    fsm_eventSingUp(&demoSM, a, b, pressB);
    fsm_eventSingUp(&demoSM, a, c, pressC);
    fsm_eventSingUp(&demoSM, b, d, pressD);
    fsm_eventSingUp(&demoSM, c, a, pressA);
    fsm_eventSingUp(&demoSM, d, a, pressA);
    fsm_eventSingUp(&demoSM, d, b, pressB);
}

/*
包装一下 fsm_run 函数，以便在引用时更直观
*/
void smDemoRun(void)
{
    fsm_run(&demoSM);
}