#include "stdio.h"
#include "stdlib.h"
#include "stateMachine.h"

//标记当前的状态
static stateMachine_stateID_t currentStateID = 0;

/*
初始化状态机
*/
void fsm_init(stateMachine_stateID_t defaultState)
{
    currentStateID = defaultState;

    pStateMachine = (stateMachine_t *)malloc(sizeof(stateMachine_t) * stateID_end);

    //遍历数组,将其每一个状态的状态ID设置为数组的序号,这与 stateMachine_stateID_t 的定义是一致的
    for(int i=0; i < stateID_end; i++)
    {
        pStateMachine[i].stateID = i;
        pStateMachine[i].stateID_l = i;
        pStateMachine[i].actions.pDoAction = NULL;
        pStateMachine[i].actions.pEnterAction = NULL;
        pStateMachine[i].actions.pExistAction = NULL;
        pStateMachine[i].events = NULL;
        pStateMachine[i].roundCounter = 0;
    }
}

/*
事件注册函数,将指定的事件注册到对应的状态下,但需要注意:
事件的执行由先向后,所以注册事件时,请将高优先级的事件先行注册,低优先级的事件后注册
*/
void fsm_eventSingUp(stateMachine_stateID_t state, stateEvent pEvent, stateMachine_stateID_t nextState)
{
    //如果 pStateMachine 没有初始化, 无法注册事件,直接返回
    if (IS_NULL(pStateMachine)){return;}
    
    //如果要向 stateID_end 注册事件,这是不被允许的
    if (stateID_end <= state){return;}
    
    if (IS_NULL(pStateMachine[state].events))
    {
        pStateMachine[state].events = (stateMachine_event_t*)malloc(sizeof(stateMachine_event_t));
        pStateMachine[state].events->pEventForGoing = pEvent;
        pStateMachine[state].events->nextState = nextState;
        pStateMachine[state].events->nextEvent = NULL;
    }
    else
    {
        stateMachine_event_t *p = (stateMachine_event_t*)malloc(sizeof(stateMachine_event_t));
        p->pEventForGoing = pEvent;
        p->nextState = nextState;
        p->nextEvent = NULL;
        for (stateMachine_event_t *idx = pStateMachine[state].events;;idx = idx->nextEvent)
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
    //如果 pStateMachine 没有初始化, 无法注册动作,直接返回
    if (IS_NULL(pStateMachine)){return;}

    //如果要向 stateID_end 注册事件,这是不被允许的
    if (stateID_end <= state){return;}
    
    pStateMachine[state].actions.pEnterAction = pEnter;
    pStateMachine[state].actions.pDoAction = pDo;
    pStateMachine[state].actions.pExistAction = pExist;
}

void fsm_run()      //运行一次状态机
{   
    //获取当前的状态单元
    stateMachine_t *pSm = &pStateMachine[currentStateID];
    stateMachine_t *pSmNew = NULL;

    // 执行当前状态的逗留活动
    if(IS_NULL(*pSm->actions.pDoAction)) {return;}
    else {pSm->actions.pDoAction(pSm);}

    //如果这个状态没有定义任何事件,则返回
    if(IS_NULL(pSm->events)){return;}

    //轮询当前状态的的事件
    for(stateMachine_event_t *p = pSm->events;!IS_NULL(p); p=p->nextEvent)
    {
        // 如果这个事件存在目标状态(stateID_end 不被识为有效的目标状态)
        if(stateID_end > p->nextState)
        {
            if(!IS_NULL(p->pEventForGoing) && go == p->pEventForGoing(pSm))
            {
                //找到要跳转的目标状态
                pSmNew = &pStateMachine[p->nextState];
                    
                //更新 stateID_l 值
                pSmNew->stateID_l = pSm->stateID;

                //更新状态ID
                currentStateID = pSmNew->stateID;

                //结束事件循环
                break;
            }
        }
    }

    //如果进入了新的状态
    if(!IS_NULL(pSmNew))
    {
        //执行前一状态的 exist 动作
        if(!IS_NULL(*pSm->actions.pExistAction)) {pSm->actions.pExistAction(pSm);}

        //执行本状态的 Enter 动作
        if(!IS_NULL(*pSmNew->actions.pEnterAction)) {pSmNew->actions.pEnterAction(pSmNew);}
    }
}