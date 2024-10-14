#ifndef AEB98F67_FAB6_4295_BABC_018F687F4757
#define AEB98F67_FAB6_4295_BABC_018F687F4757
#include "stateMachine.h"

/*
这是一个状态机的demo，演示了如何在 stateMachine.h 的基础上构建一个有实际用途的状态机
大致你需要做以下工作：
0、你需要把 stateMachine.h 和 stateMachine.c 两个文件复制到你的工作项目中，并引用到你的状机 .h 文档中
1、为了方便管理状态机的各个状态，建议你定义一个类似 demoStateID_t 的enum类型，如下示例
2、定义 stateMachine_t 变量，这个变量用于统一管理状态机
3、你需要为你的状态定义 stateAction 类型的 enter，do， exist 三个动作/活动，在这些活动中，你可以根据需要设置不同的flg，或者其它状态值
    3.1、如果你需要在不同的状态下，使用不同的 enter, do, exist 动作，你可以分别定义
    3.2、如果你的状态不需要做什么动作，你也可以不用定义这些动作，不给这些状态注册动作
4、你需要定义一些 eventFunc 类型的事件，当这些事件发生时，可以控制状态进行跳转
5、你需要实现一个类似 smDemoBuild 的函数，这个函数实现以下功能：
    5.1、调用 fsm_init() 方法初始化状态机，并设置默认状态
    5.2、根据你的需要，为你的每一个状态注册动作/活动
    5.3、根据你的需要，为你的每一个状态注册事件
        5.3.1、如果你的某一个状态没有注册任何事件，那么这表示一旦状态跳转到这个状态中后，就再也不会跳出了，请谨慎处理这种情况
    5.4、5.2和5.3不分先后
6、实现一个类似 smRun 的函数，这个函数将 fsm_run 包装一层，以便在你的应用中调用更加直观
7、如果你有需要，你可以定义多个 stateMachine_t 变量，实现状态机的嵌套，但这你需要在你的状态的合适的动作中安排运行子状态

做完以上，你应该可以试试你的状态机效果了 ^_^
*/

//声明4个状态机必要变量
typedef enum
{   
    //请将你的状态依次写到这里,请保证状态值从0开始且是连续的
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    
    // 这是状态结束标记，为了方便你管理状态的最大值
    stateID_end,
} demoState_t;

stateMachine_t demoSM;

void actionEntry(stateMachineUnit_t *pSm);
void actionDo(stateMachineUnit_t *pSm);
void actionExit(stateMachineUnit_t *pSm);

stateMachine_eventResult_t pressA(stateMachineUnit_t *pSm);
stateMachine_eventResult_t pressB(stateMachineUnit_t *pSm);
stateMachine_eventResult_t pressC(stateMachineUnit_t *pSm);
stateMachine_eventResult_t pressD(stateMachineUnit_t *pSm);

void smDemoBuild(void);
void smDemoRun(void);

char inputKey;

#endif /* AEB98F67_FAB6_4295_BABC_018F687F4757 */
