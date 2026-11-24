#include "smDemo.h"

#include <stdio.h>

char keys[7] = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};

char inputKey;
stateMachine_t SM_MEM demoSM;

void actionEntry(smUnitPtr_t pSt) {
    if (pSt->pSm->roundCounter == 0) {
        // 状态机首次运行时
        printf("the current state is: %c\n", keys[pSt->stateID]);
    } else {
        // 状态机切换进入某一状态时
        printf(" --> %c\n", keys[pSt->stateID]);
    }
}

void actionDo(smUnitPtr_t pSt) {
    // 状态机的do事件
    printf("----roundCounter of %c is %lu\n", keys[pSt->stateID], (unsigned long)pSt->roundCounter);
}

void actionExit(smUnitPtr_t pSt) {
    // 状态机的退出事件
    printf("state exchanged: %c", keys[pSt->stateID]);
}

smEventResult_t pressA(smUnitPtr_t pSt) { return 'a' == inputKey; };
smEventResult_t pressB(smUnitPtr_t pSt) { return 'b' == inputKey; };
smEventResult_t pressC(smUnitPtr_t pSt) { return 'c' == inputKey; };
smEventResult_t pressD(smUnitPtr_t pSt) { return 'd' == inputKey; };

void smDemoBuild() {
    // 第 4 个参数是异常报警回调，第 5 个参数是每个 do 动作之后要执行的动作，本例都不用，传 0
    fsm_init(&demoSM, stateID_count, 0, 0, 0);

    // 注册状态动作
    // 为每一个状态指定进入事件，逗留事件和退出事件
    // 如果你的某些状态不需要全部的事件，则将不需要对应事件的位置传 0 即可
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

    // 状态机配置完成后，DMEM_BUFFER_SIZE 的取值请按 `bufferUsed` 的实际值（或 map 里 DMEMORY 的占用）来定
}

/*
包装一下 fsm_run 函数，以便在引用时更直观
*/
void smDemoRun(void) {
    fsm_run(&demoSM);
}