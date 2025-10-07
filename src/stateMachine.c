#include "stateMachineMemmory.h"
#include "stateMachine.h"

DMEM *dyMM;

static void __reset(stateMachine_t *pSm);
static void __eventSingUp(stateMachine_t *pSm, uint8_t stateID, uint8_t nextState, smEventResult_t (*pEvent)(smUnit_t *));
static void __actionSignUp(stateMachine_t *pSm, uint8_t stateID, void (*pEnter)(smUnit_t *), void (*pDo)(smUnit_t *), void (*pExist)(smUnit_t *));
static void __run(stateMachine_t *pSm);

/*
初始化状态机
*/
void fsm_init(stateMachine_t *pSm, uint8_t stateIDs_count, uint8_t stateID_default)
{
	pSm->stateID_default = stateID_default;
	pSm->stateIDs_Count = stateIDs_count;
	pSm->stateID = pSm->stateID_default;
	pSm->roundCounter = 0;
	dyMM = DynMemGet((sizeof(uint32_t) * pSm->stateIDs_Count));
	if (IS_pSafe(dyMM)){
		pSm->enterCounterOf = (uint32_t *)dyMM->addr;
	}else{
		while(1); //如果内存分配不成功，则死在这里
	}
	
	pSm->latched = false;
	#if defined(SM_BUFFER_FULL) || defined(SM_BUFFER_PART) || defined(SM_BUFFER_TINY)
	pSm->buffer = {0};				//初始化状态机的buffer
	pSm->buffer.ptr = NULL;		//初始化状态机的buffer.ptr指针为NULL
	#endif

	dyMM = DynMemGet(sizeof(smUnit_t) * pSm->stateIDs_Count);
	if(IS_pSafe(dyMM)){
		pSm->pSMChain = (smUnit_t *)dyMM->addr;
	}else{
		while(1); //如果内存分配不成功，则死在这里
	}

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
		pSm->pSMChain[i].pSm = pSm;							//登记状态机的指针
		#if defined(ST_BUFFER_FULL) || defined(ST_BUFFER_PART) || defined(ST_BUFFER_TINY)
		pSm->pSMChain[i].buffer = {0};				//初始化各状态的buffer
		pSm->pSMChain[i].buffer.ptr = NULL;			//初始化各状态的buffer.ptr指针为NULL
		#endif

		//初始化内部变量
		pSm->pSMChain[i].roundCounter = 0;

		//同步设置该状态的出现次数为0
		pSm->enterCounterOf[i] = 0;
	}

	pSm->reset = __reset;
	pSm->eventSingUp = __eventSingUp;
	pSm->actionSignUp = __actionSignUp;
	pSm->run = __run;

	pSm->actionOnChangeBeforeEnter = NULL;
	pSm->actionAfterDo = NULL;
}

/*
将指定的状态机，复位到默认的状态
*/
static void __reset(stateMachine_t *pSm)
{
	if(IS_pSafe(pSm) && IS_pSafe(pSm->pSMChain)){
		smUnit_t *st = &pSm->pSMChain[pSm->stateID];

		st->latched = false;													//解除当前状态的状态锁
		if(IS_pSafe(st->actions.pExistAction)) {st->actions.pExistAction(st);}	//执行当前状的退出事件
		//考虑到状态机复位后，状态机未必能及时轮询运行（例如子状态的状态机，依懒于父状态机的轮询调用），所以：
		//新状态的 enter 事件，将在状态机轮询时执行，这样可以保障状态机执行是连续的
		
		//复位状态机
		pSm->roundCounter = 0;	//复位状态机的轮询次数
		pSm->stateID = pSm->stateID_default;

		//复位各状态出现的次数值
		for(int i=0; i < pSm->stateIDs_Count; i++)
		{
			pSm->enterCounterOf[i] = 0;
			pSm->pSMChain[i].stateID_l = pSm->stateIDs_Count;
			pSm->pSMChain[i].latched = false;
		}
	}
}

/*
向指定的状态机注册事件,将指定的事件注册到对应的状态下,但需要注意:
事件的执行由先向后,所以注册事件时,请将高优先级的事件先行注册,低优先级的事件后注册
*/
static void __eventSingUp(stateMachine_t *pSm, uint8_t stateID, uint8_t nextState, smEventResult_t (*pEvent)(smUnit_t *))
{
	//如果 __pStateMachine 没有初始化, 无法注册事件,直接返回
	if (IS_NULL(pSm)||IS_NULL(pSm->pSMChain)){return;}
	
	//如果要注册的stateID不合理，则退出
	if (pSm->stateIDs_Count <= stateID){return;}
	
	dyMM = DynMemGet(sizeof(struct stateMachine_event_s));
	if(!IS_pSafe(dyMM)){
		while(1); //如果内存分配不成功，则死在这里
	}

	if(IS_NULL(pSm->pSMChain[stateID].events))
	{
		pSm->pSMChain[stateID].events = (struct stateMachine_event_s *)dyMM->addr;
		pSm->pSMChain[stateID].events->pEventForGoing = pEvent;
		pSm->pSMChain[stateID].events->nextState = nextState;
		pSm->pSMChain[stateID].events->nextEvent = NULL;
	} else {
		struct stateMachine_event_s *p = (struct stateMachine_event_s *)dyMM->addr;
		p->pEventForGoing = pEvent;
		p->nextState = nextState;
		p->nextEvent = NULL;
		for (struct stateMachine_event_s *stEvent = pSm->pSMChain[stateID].events;;stEvent = stEvent->nextEvent)
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
static void __actionSignUp(stateMachine_t *pSm, uint8_t stateID, void (*pEnter)(smUnit_t *), void (*pDo)(smUnit_t *), void (*pExist)(smUnit_t *))
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
static void __run(stateMachine_t *pSm)
{
	//如果状态机或者状态链没有初始化, 无法注册动作,直接返回
	if (IS_NULL(pSm)){return;}

	pSm->roundCounter++;
	if(pSm->latched){//如果状态机被锁，则不运行任何实际逻辑
		return;
	}

	//获取当前的状态单元
	smUnit_t *st = &pSm->pSMChain[pSm->stateID];
	smUnit_t *stNew = NULL;

	//更新当前状态的计数值
	st->roundCounter++;
	
	//如果是第一次轮询状态机，则需要先执行一次Enter动作 和Do动作
	if (pSm->stateIDs_Count == st->stateID_l){
		pSm->roundCounter = 0;		//状态状态机计数
		st->roundCounter = 0;		//复位状态计数
		st->stateID_l = st->stateID;
		if(IS_pSafe(pSm->actionOnChangeBeforeEnter)) {pSm->actionOnChangeBeforeEnter(st);}	//如果注册有状态切换事件，则执行之
		if(IS_pSafe(st->actions.pEnterAction)) {st->actions.pEnterAction(st);}	//如果有enter事件，则执行之
		if(IS_pSafe(st->actions.pDoAction)) {st->actions.pDoAction(st);}		//如果有do事件，则执行之
		if(IS_pSafe(pSm->actionAfterDo)) {pSm->actionAfterDo(st);}				//如果有actionAfterDo事件，则执行之
	}else{
		//如果这个状态有定义事件，并且没有被锁，则检测跳转事件是否发生
		if(IS_pSafe(st->events) && !st->latched){
			//轮询当前状态的的事件
			for(struct stateMachine_event_s *p = st->events;IS_pSafe(p); p=p->nextEvent)
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
			if(IS_pSafe(pSm->actionOnChangeBeforeEnter)) {pSm->actionOnChangeBeforeEnter(stNew);}	//如果注册有状态切换事件，则执行之
			if(IS_pSafe(stNew->actions.pEnterAction)) {stNew->actions.pEnterAction(stNew);}			//执行新状态的 enter 动作
			if(IS_pSafe(stNew->actions.pDoAction)) {stNew->actions.pDoAction(stNew);}				//执行新状态的 do 动作
			if(IS_pSafe(pSm->actionAfterDo)) {pSm->actionAfterDo(stNew);}							//如果有actionAfterDo事件，则执行之
		}else{//如果继续留在当前状态，则执行当前状态的逗留活动
			//执行本状态的逗留活动
			if(IS_pSafe(st->actions.pDoAction)) {st->actions.pDoAction(st);}
			if(IS_pSafe(pSm->actionAfterDo)) {pSm->actionAfterDo(st);}				//如果有actionAfterDo事件，则执行之
		}
	}
}