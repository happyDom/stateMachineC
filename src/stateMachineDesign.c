#include <stdio.h>
#include "limits.h"
#include "stateMachineDesign.h"

char keys[7] = {'a','b','c','d','e','f','g'};

void actionEntry(stateMachine_t *pSm)
{
    pSm->roundCounter = 0;
    printf("%c is pressed, state enter  to: %c\n", inputKey, keys[pSm->stateID]);
}

void actionDo(stateMachine_t *pSm)
{
    if(pSm->roundCounter < UINT_MAX)
        pSm->roundCounter++;
    
    printf("roundCounter of %c is %d\n", keys[pSm->stateID], pSm->roundCounter);
}

void actionExit(stateMachine_t *pSm)
{
    printf("%c is pressed, state exist from: %c\n", inputKey, keys[pSm->stateID_l]);
}

stateMachine_eventResult_t pressA(stateMachine_t *pSm) {return 'a' == inputKey;};
stateMachine_eventResult_t pressB(stateMachine_t *pSm) {return 'b' == inputKey;};
stateMachine_eventResult_t pressC(stateMachine_t *pSm) {return 'c' == inputKey;};
stateMachine_eventResult_t pressD(stateMachine_t *pSm) {return 'd' == inputKey;};

void dyyStateMachineBuild(stateMachine_stateID_t defaultState)
{
    fsm_init(defaultState);
    
    // 注册状态动作
    fsm_actionSignUp(a,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(b,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(c,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    fsm_actionSignUp(d,(stateAction)&actionEntry, (stateAction)&actionDo, (stateAction)&actionExit);
    
    // 注册状态事件
    fsm_eventSingUp(a, (stateEvent)&pressB, b);
    fsm_eventSingUp(a, (stateEvent)&pressC, c);
    fsm_eventSingUp(b, (stateEvent)&pressD, d);
    fsm_eventSingUp(c, (stateEvent)&pressA, a);
    fsm_eventSingUp(d, (stateEvent)&pressA, a);
    fsm_eventSingUp(d, (stateEvent)&pressB, b);
}

void dyyStateMachineRun(void)
{
    fsm_run();
}