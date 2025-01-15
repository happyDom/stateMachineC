#include "stateMachine.h"
#include <stdio.h>
#include <stdlib.h>

/*
初始化状态机
*/
void fsm_init(stateMachine_t *pSm, uint8_t stateIDs_count, uint8_t stateID_default)
{
	pSm->stateID_default = stateID_default;
	pSm->stateIDs_Count = stateIDs_count;
	pSm->stateID = pSm->stateID_default;
	pSm->roundCounter = 0;
	pSm->enterCounterOf = (uint32_t *)malloc(sizeof(uint32_t) * pSm->stateIDs_Count);
	pSm->buffer = NULL;
	pSm->latched = false;

	pSm->pSMChain = (stateMachineUnit_t *)malloc(sizeof(stateMachineUnit_t) * pSm->stateIDs_Count);

	//遍历数组,将其每一个状态的状态ID设置为数组的序号,这与 unsigned int 的定义是一致的
	for(int i=0; i < pSm->stateIDs_Count; i++)
	{
		pSm->pSMChain[i].stateID = i;
		pSm->pSMChain[i].stateID_l = pSm->stateIDs_Count;	 //默认的前一状态为 stateID_end
		pSm->pSMChain[i].latched = false;
		pSm->pSMChain[i].actions.pDoAction = NULL;
		pSm->pSMChain[i].actions.pEnterAction = NULL;
		pSm->pSMChain[i].actions.pExistAction = NULL;
		pSm->pSMChain[i].events = NULL;
		pSm->pSMChain[i].buffer = NULL;
		pSm->pSMChain[i].pSm = pSm;							//登记状态机的指针

		//初始化内部变量
		pSm->pSMChain[i].roundCounter = 0;

		//同步设置该状态的出现次数为0
		pSm->enterCounterOf[i] = 0;
	}
}

/*
将指定的状态机，复位到默认的状态
*/
void fsm_reset(stateMachine_t *pSm)
{
	if(IS_pSafe(pSm) && IS_pSafe(pSm->pSMChain)){
		stateMachineUnit_t *st = &pSm->pSMChain[pSm->stateID];

		st->latched = false;													//解除当前状态的状态锁
		if(IS_pSafe(st->actions.pExistAction)) {st->actions.pExistAction(st);}	//执行当前状的退出事件
		
		//考虑到状态机复位后，状态机未必能及时轮询运行（例如子状态的状态机，依懒于父状态机的轮询调用），所以：
		//新状态的 enter 事件，将在状态机轮询时执行，这样可以保障状态机执行是连续的
		
		//复位状态机
		pSm->roundCounter = 0;	//复位状态机的轮询次数
		pSm->stateID = pSm->stateID_default;
		pSm->pSMChain[pSm->stateID_default].stateID_l = pSm->stateIDs_Count;
		pSm->pSMChain[pSm->stateID_default].latched = false;					//复位状态锁

		//复位各状态出现的次数值
		for(int i=0; i < pSm->stateIDs_Count; i++)
		{
			pSm->enterCounterOf[i] = 0;
		}
	}
}

/*
向指定的状态机注册事件,将指定的事件注册到对应的状态下,但需要注意:
事件的执行由先向后,所以注册事件时,请将高优先级的事件先行注册,低优先级的事件后注册
*/
void fsm_eventSingUp(stateMachine_t *pSm, uint8_t stateID, uint8_t nextState, stateMachine_eventResult_t (*pEvent)(stateMachineUnit_t *))
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
void fsm_actionSignUp(stateMachine_t *pSm, uint8_t stateID, void (*pEnter)(stateMachineUnit_t *), void (*pDo)(stateMachineUnit_t *), void (*pExist)(stateMachineUnit_t *))
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
	//如果状态机或者状态链没有初始化, 无法注册动作,直接返回
	if (IS_NULL(pSm)){return;}

	if(pSm->latched){//如果状态机被锁，则只增加计数器，不运行任何实际逻辑
		pSm->roundCounter++;
		return;
	}
	//获取当前的状态单元
	stateMachineUnit_t *st = &pSm->pSMChain[pSm->stateID];
	stateMachineUnit_t *stNew = NULL;

	//如果是第一次轮询状态机，则需要先执行一次Enter动作 和Do动作
	if (pSm->stateIDs_Count == st->stateID_l){
		st->roundCounter = 0;		//复位状态计数
		st->stateID_l = st->stateID;
		if(IS_pSafe(st->actions.pEnterAction)) {st->actions.pEnterAction(st);}	//如果有enter事件，则执行之
		if(IS_pSafe(st->actions.pDoAction)) {st->actions.pDoAction(st);}		//如果有do事件，则执行之
	}else{
		//如果这个状态有定义事件，并且没有被锁，则检测跳转事件是否发生
		if(IS_pSafe(st->events) && !st->latched){
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
						}

						//结束事件循环
						break;
					}
				}
			}
		}

		//更新状态机的计数值
		pSm->roundCounter++;

		//更新当前状态的计数值
		st->roundCounter++;

		//如果进入了新的状态
		if(IS_pSafe(stNew))
		{
			//将新状态的 stateID_l 值更新为当前状态的 stateID 值
			stNew->stateID_l = st->stateID;

			//更新状态ID
			pSm->stateID = stNew->stateID;
			
			//执行当前状态的 exist 动作
			if(IS_pSafe(st->actions.pExistAction)) {st->actions.pExistAction(st);}

			//更新当前状态的出现次数
			pSm->enterCounterOf[st->stateID]++;

			stNew->roundCounter = 0;	//复位新状态计数器
			if(IS_pSafe(stNew->actions.pEnterAction)) {stNew->actions.pEnterAction(stNew);}	//执行新状态的 enter 动作
			if(IS_pSafe(stNew->actions.pDoAction)) {stNew->actions.pDoAction(stNew);}	//执行新状态的 do 动作
		}else{//如果继续留在当前状态，则执行当前状态的逗留活动
			//执行本状态的逗留活动
			if(IS_pSafe(st->actions.pDoAction)) {st->actions.pDoAction(st);}
		}
	}
}
