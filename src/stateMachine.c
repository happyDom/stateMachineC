#include "stdio.h"
#include "stdlib.h"
#include "stateMachine.h"

/*
初始化状态机
*/
void fsm_init(stateMachine_stateID_t defaultState)
{
    __currentStateID = defaultState;

    __pStateMachine = (stateMachineUnit_t *)malloc(sizeof(stateMachineUnit_t) * stateID_end);

    //遍历数组,将其每一个状态的状态ID设置为数组的序号,这与 stateMachine_stateID_t 的定义是一致的
    for(int i=0; i < stateID_end; i++)
    {
        __pStateMachine[i].stateID = i;
        __pStateMachine[i].stateID_l = i;
        __pStateMachine[i].actions.pDoAction = NULL;
        __pStateMachine[i].actions.pEnterAction = NULL;
        __pStateMachine[i].actions.pExistAction = NULL;
        __pStateMachine[i].events = NULL;

        //初始化内部变量
        __pStateMachine[i].roundCounter = 0;
    }
}

/*
事件注册函数,将指定的事件注册到对应的状态下,但需要注意:
事件的执行由先向后,所以注册事件时,请将高优先级的事件先行注册,低优先级的事件后注册
*/
void fsm_eventSingUp(stateMachine_stateID_t state, stateMachine_stateID_t nextState, eventFunc pEvent)
{
    //如果 __pStateMachine 没有初始化, 无法注册事件,直接返回
    if (IS_NULL(__pStateMachine)){return;}
    
    //如果要向 stateID_end 注册事件,这是不被允许的
    if (stateID_end <= state){return;}
    
    if (IS_NULL(__pStateMachine[state].events))
    {
        __pStateMachine[state].events = (stateMachine_event_t*)malloc(sizeof(stateMachine_event_t));
        __pStateMachine[state].events->pEventForGoing = pEvent;
        __pStateMachine[state].events->nextState = nextState;
        __pStateMachine[state].events->nextEvent = NULL;
    }
    else
    {
        stateMachine_event_t *p = (stateMachine_event_t*)malloc(sizeof(stateMachine_event_t));
        p->pEventForGoing = pEvent;
        p->nextState = nextState;
        p->nextEvent = NULL;
        for (stateMachine_event_t *idx = __pStateMachine[state].events;;idx = idx->nextEvent)
        {
            if(IS_NULL(idx->nextEvent))
            {
                idx->nextEvent = p;
                break;
            }
        }
    }
}

/*
动作注册函数,将指定的事件注册到对应的状态下
*/
void fsm_actionSignUp(stateMachine_stateID_t state, stateAction pEnter, stateAction pDo, stateAction pExist)
{
    //如果 __pStateMachine 没有初始化, 无法注册动作,直接返回
    if (IS_NULL(__pStateMachine)){return;}

    //如果要向 stateID_end 注册事件,这是不被允许的
    if (stateID_end <= state){return;}
    
    __pStateMachine[state].actions.pEnterAction = pEnter;
    __pStateMachine[state].actions.pDoAction = pDo;
    __pStateMachine[state].actions.pExistAction = pExist;
}

void fsm_run()      //运行一次状态机
{
    //获取当前的状态单元
    stateMachineUnit_t *pSm = &__pStateMachine[__currentStateID];
    stateMachineUnit_t *pSmNew = NULL;

    // 执行当前状态的逗留活动
    if(IS_NULL(pSm->actions.pDoAction)) {return;}
    else {pSm->actions.pDoAction(pSm);}

    //如果这个状态没有定义任何事件,则返回
    if(IS_NULL(pSm->events)){return;}

    //轮询当前状态的的事件
    for(stateMachine_event_t *p = pSm->events;IS_pSafe(p); p=p->nextEvent)
    {
        // 如果这个事件存在目标状态(stateID_end 不被识为有效的目标状态)
        if(stateID_end > p->nextState)
        {
            if(IS_pSafe(p->pEventForGoing) && go == p->pEventForGoing(pSm))
            {
                //找到要跳转的目标状态
                pSmNew = &__pStateMachine[p->nextState];
                    
                //更新 stateID_l 值
                pSmNew->stateID_l = pSm->stateID;

                //更新状态ID
                __currentStateID = pSmNew->stateID;

                //结束事件循环
                break;
            }
        }
    }

    //如果进入了新的状态
    if(IS_pSafe(pSmNew))
    {
        //执行前一状态的 exist 动作
        if(IS_pSafe(pSm->actions.pExistAction)) {pSm->actions.pExistAction(pSm);}

        //执行本状态的 Enter 动作
        if(IS_pSafe(pSmNew->actions.pEnterAction)) {pSmNew->actions.pEnterAction(pSmNew);}
    }
}