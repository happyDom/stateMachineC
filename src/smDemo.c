#include <stdio.h>
#include "limits.h"
#include "smDemo.h"

char keys[7] = {'a','b','c','d','e','f','g'};

void actionEntry(stateMachineUnit_t *pSm)
{
    pSm->roundCounter = 0;
    printf("%c is pressed, state enter  to: %c\n", inputKey, keys[pSm->stateID]);
}

void actionDo(stateMachineUnit_t *pSm)
{
    if(pSm->roundCounter < UINT_MAX)
        pSm->roundCounter++;
    
    printf("roundCounter of %c is %d\n", keys[pSm->stateID], pSm->roundCounter);
}

void actionExit(stateMachineUnit_t *pSm)
{
    printf("%c is pressed, state exist from: %c\n", inputKey, keys[pSm->stateID]);
}

stateMachine_eventResult_t pressA(stateMachineUnit_t *pSm) {return 'a' == inputKey;};
stateMachine_eventResult_t pressB(stateMachineUnit_t *pSm) {return 'b' == inputKey;};
stateMachine_eventResult_t pressC(stateMachineUnit_t *pSm) {return 'c' == inputKey;};
stateMachine_eventResult_t pressD(stateMachineUnit_t *pSm) {return 'd' == inputKey;};

void smDemoBuild(stateMachine_stateID_t defaultState)
{
    fsm_init(defaultState);
    
    // 注册状态动作
    fsm_actionSignUp(a,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(b,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(c,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(d,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    
    // 注册状态事件
    fsm_eventSingUp(a, b, (eventFunc)&pressB);
    fsm_eventSingUp(a, c, (eventFunc)&pressC);
    fsm_eventSingUp(b, d, (eventFunc)&pressD);
    fsm_eventSingUp(c, a, (eventFunc)&pressA);
    fsm_eventSingUp(d, a, (eventFunc)&pressA);
    fsm_eventSingUp(d, b, (eventFunc)&pressB);
}

/*
包装一下 fsm_run 函数，以便在引用时更直观
*/
void smDemoRun(void)
{
    fsm_run();
}