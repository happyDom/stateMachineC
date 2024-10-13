#ifndef AEB98F67_FAB6_4295_BABC_018F687F4757
#define AEB98F67_FAB6_4295_BABC_018F687F4757
#include "stateMachine.h"

char inputKey;

void actionEntry(stateMachine_t *pSm);
void actionDo(stateMachine_t *pSm);
void actionExit(stateMachine_t *pSm);

stateMachine_eventResult_t pressA(stateMachine_t *pSm);
stateMachine_eventResult_t pressB(stateMachine_t *pSm);
stateMachine_eventResult_t pressC(stateMachine_t *pSm);
stateMachine_eventResult_t pressD(stateMachine_t *pSm);

void dyyStateMachineBuild(stateMachine_stateID_t defaultState);
void dyyStateMachineRun(void);

#endif /* AEB98F67_FAB6_4295_BABC_018F687F4757 */
