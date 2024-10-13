#ifndef AEB98F67_FAB6_4295_BABC_018F687F4757
#define AEB98F67_FAB6_4295_BABC_018F687F4757
#include "stateMachine.h"

char inputKey;

void actionEntry(stateMachineUnit_t *pSm);
void actionDo(stateMachineUnit_t *pSm);
void actionExit(stateMachineUnit_t *pSm);

stateMachine_eventResult_t pressA(stateMachineUnit_t *pSm);
stateMachine_eventResult_t pressB(stateMachineUnit_t *pSm);
stateMachine_eventResult_t pressC(stateMachineUnit_t *pSm);
stateMachine_eventResult_t pressD(stateMachineUnit_t *pSm);

void dyyStateMachineBuild(stateMachine_stateID_t defaultState);
void dyyStateMachineRun(void);

#endif /* AEB98F67_FAB6_4295_BABC_018F687F4757 */
