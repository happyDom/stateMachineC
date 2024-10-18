#include <stdio.h>
#include "smDemo.h"

char keys[7] = {'a','b','c','d','e','f','g'};

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
    
    // 注册状态动作
    fsm_actionSignUp(&demoSM, a, (stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(&demoSM, b,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(&demoSM, c,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(&demoSM, d,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    
    // 注册状态事件
    fsm_eventSingUp(&demoSM, a, b, (eventFunc)&pressB);
    fsm_eventSingUp(&demoSM, a, c, (eventFunc)&pressC);
    fsm_eventSingUp(&demoSM, b, d, (eventFunc)&pressD);
    fsm_eventSingUp(&demoSM, c, a, (eventFunc)&pressA);
    fsm_eventSingUp(&demoSM, d, a, (eventFunc)&pressA);
    fsm_eventSingUp(&demoSM, d, b, (eventFunc)&pressB);
}

/*
包装一下 fsm_run 函数，以便在引用时更直观
*/
void smDemoRun(void)
{
    fsm_run(&demoSM);
}