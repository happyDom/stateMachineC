#include "stateMachine.h"

/*
初始化状态机
*/
void fsm_init(stateMachine_t *pSm, uint8_t stateIDs_count, uint8_t stateID_default)
{
    pSm->stateID_default = stateID_default;
    pSm->stateIDs_Count = stateIDs_count;
    pSm->stateID = pSm->stateID_default;

    pSm->pSMChain = (stateMachineUnit_t *)malloc(sizeof(stateMachineUnit_t) * pSm->stateIDs_Count);

    //遍历数组,将其每一个状态的状态ID设置为数组的序号,这与 unsigned int 的定义是一致的
    for(int i=0; i < pSm->stateIDs_Count; i++)
    {
        pSm->pSMChain[i].stateID = i;
        pSm->pSMChain[i].stateID_l = pSm->stateIDs_Count;     //默认的前一状态为 stateID_end
        pSm->pSMChain[i].latch = released;
        pSm->pSMChain[i].actions.pDoAction = NULL;
        pSm->pSMChain[i].actions.pEnterAction = NULL;
        pSm->pSMChain[i].actions.pExistAction = NULL;
        pSm->pSMChain[i].events = NULL;
        pSm->pSMChain[i].buffer = NULL;

        //初始化内部变量
        pSm->pSMChain[i].roundCounter = 0;
    }
}

/*
将指定的状态机，复位到默认的状态
*/
void fsm_reset(stateMachine_t *pSm)
{
    if(IS_pSafe(pSm) && IS_pSafe(pSm->pSMChain)){
        pSm->stateID = pSm->stateID_default;
        pSm->pSMChain[pSm->stateID].stateID_l = pSm->stateIDs_Count;
    }
}

/*
向指定的状态机注册事件,将指定的事件注册到对应的状态下,但需要注意:
事件的执行由先向后,所以注册事件时,请将高优先级的事件先行注册,低优先级的事件后注册
*/
void fsm_eventSingUp(stateMachine_t *pSm, uint8_t stateID, uint8_t nextState, eventFunc pEvent)
{
    //如果 __pStateMachine 没有初始化, 无法注册事件,直接返回
    if (IS_NULL(pSm)||IS_NULL(pSm->pSMChain)){return;}
    
    //如果要注册的stateID不合理，则退出
    if (pSm->stateIDs_Count <= stateID){return;}
    
    if(IS_NULL(pSm->pSMChain[stateID].events))
    {
        pSm->pSMChain[stateID].events = (stateMachine_event_t*)malloc(sizeof(stateMachine_event_t));
        pSm->pSMChain[stateID].events->pEventForGoing = pEvent;
        pSm->pSMChain[stateID].events->nextState = nextState;
        pSm->pSMChain[stateID].events->nextEvent = NULL;
    }
    else
    {
        stateMachine_event_t *p = (stateMachine_event_t*)malloc(sizeof(stateMachine_event_t));
        p->pEventForGoing = pEvent;
        p->nextState = nextState;
        p->nextEvent = NULL;
        for (stateMachine_event_t *stEvent = pSm->pSMChain[stateID].events;;stEvent = stEvent->nextEvent)
        {
            if(IS_NULL(stEvent->nextEvent))
            {
                stEvent->nextEvent = p;
                break;
            }
        }
    }
}

/*
向指定的状态机注册动作,将指定的事件注册到对应的状态下
*/
void fsm_actionSignUp(stateMachine_t *pSm, uint8_t stateID, stateAction pEnter, stateAction pDo, stateAction pExist)
{
    //如果状态机或者状态链没有初始化, 无法注册动作,直接返回
    if (IS_NULL(pSm) || IS_NULL(pSm->pSMChain)){return;}

    //如果要注册的stateID不合理，则退出
    if (pSm->stateIDs_Count <= stateID){return;}
    
    pSm->pSMChain[stateID].actions.pEnterAction = pEnter;
    pSm->pSMChain[stateID].actions.pDoAction = pDo;
    pSm->pSMChain[stateID].actions.pExistAction = pExist;
}

/*
运行指定的状态机
*/
void fsm_run(stateMachine_t *pSm)
{
    //获取当前的状态单元
    stateMachineUnit_t *st = &pSm->pSMChain[pSm->stateID];
    stateMachineUnit_t *stNew = NULL;

    //如果是第一次进入状态机，则需要执行 Enter 动作
    if (pSm->stateIDs_Count == st->stateID_l){
        st->roundCounter = 0;                                               //计数器复位
        if(IS_pSafe(st->actions.pEnterAction)) {st->actions.pEnterAction(st);}
    }
    else{
        // 执行当前状态的逗留活动
        if(st->roundCounter < UINT64_MAX) {st->roundCounter++;}               //计数器 +1

        if(IS_pSafe(st->actions.pDoAction)) {st->actions.pDoAction(st);}
    }

    //如果这个状态有定义事件，并且没有被锁，则检测跳转事件是否发生
    if(IS_pSafe(st->events) && released == st->latch){
        //轮询当前状态的的事件
        for(stateMachine_event_t *p = st->events;IS_pSafe(p); p=p->nextEvent)
        {
            // 如果这个事件存在目标状态(stateID_end 不被识为有效的目标状态)
            if(pSm->stateIDs_Count > p->nextState)
            {
                if(IS_pSafe(p->pEventForGoing) && go == p->pEventForGoing(st))
                {
                    //如果跳转到了其它的状态，则将新状态赋值给 stNew
                    if(pSm->stateID != p->nextState){
                        //找到要跳转的目标状态
                        stNew = &pSm->pSMChain[p->nextState];
                        
                        //更新 stateID_l 值
                        stNew->stateID_l = st->stateID;

                        //更新状态ID
                        pSm->stateID = stNew->stateID;
                    }

                    //结束事件循环
                    break;
                }
            }
        }

        //如果进入了新的状态
        if(IS_pSafe(stNew))
        {
            //执行前一状态的 exist 动作
            if(IS_pSafe(st->actions.pExistAction)) {st->actions.pExistAction(st);}

            //执行本状态的 Enter 动作
            stNew->roundCounter = 0;    //计数器复位

            if(IS_pSafe(stNew->actions.pEnterAction)) {stNew->actions.pEnterAction(stNew);}
        }
    }

    //更新 stateID_l 值
    st->stateID_l = st->stateID;
}
