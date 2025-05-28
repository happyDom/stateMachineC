#ifndef C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define C0FD9D79_317D_44BD_BF7F_E51B5C4F850C

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define IS_NULL(p) (NULL == p)
#define IS_pSafe(p) (NULL != p)

typedef enum{
	aWait=0,
	go=1,
} stateMachine_eventResult_t;

typedef struct  {
	bool b;
	unsigned char bt[8];
	signed char i8;
    unsigned char u8;
	short s16;
	unsigned short u16;
    int i32;
    unsigned int u32;
    long long l64;
    unsigned long long u64;
    float f32;
    double d64;
	void *ptr;
} bufferUnion_t;

typedef struct stateMachineUnit_s stateMachineUnit_t;
typedef struct stateMachine_event_s stateMachine_event_t;
typedef struct stateMachine_s stateMachine_t;

typedef struct
{
	void (*pEnterAction)(stateMachineUnit_t *);
	void (*pDoAction)(stateMachineUnit_t *);
	void (*pExistAction)(stateMachineUnit_t *);
} stateMachine_actionMap_t;

struct stateMachine_event_s
{
	stateMachine_eventResult_t (*pEventForGoing)(stateMachineUnit_t *);
	uint8_t nextState;						//目标状态
	stateMachine_event_t *nextEvent;			//下一个事件
};												//这是一个单向链表,用于登记多个事件

struct stateMachineUnit_s
{
	bool latched;							//状态锁，为真时，状态机进行该状态的轮询时，不会检测该状态注册的事件
	uint8_t stateID_l;					//状态机的前一个状态
	uint8_t stateID;					//当前状态循环的状态
	stateMachine_actionMap_t actions;		//在本状态时需要执行的动作
	stateMachine_event_t *events;			//在本状态时，需要进行关注的事件，这是一个数组地址
	uint32_t roundCounter;					//这个计数器显示了在本状态期间，状态机轮询的次数，如果 1ms 轮询一次，支持最大 49.7 天时间的计数
	bufferUnion_t buffer;			//一个通用的buffer，用于存放与实际实用场景相关的数据
	stateMachine_t *pSm;					//状态机的指针，这使得状态单元可以使用状态机中的信息
};

struct stateMachine_s
{
	bool latched;					//状态机锁，为真时，状态机不运行任何状态的动作，不检测任何事件
	uint32_t *enterCounterOf;		//一个数组，用于记录状态机中每一个状态出现的次数，在对应状态退出时进行计数
	stateMachineUnit_t *pSMChain;	//存放状态单元的数组空间的地址
	void (*actionOnChangeBeforeEnter)(stateMachineUnit_t *); //状态切换前要做的动作
	void (*actionAfterDo)(stateMachineUnit_t *); //在每个do事件后执行的动作
	uint8_t stateID;				//标记当前状态机的状态
	uint8_t stateID_default;		//状态机的默认状态
	uint8_t stateIDs_Count;			//状态机的总状态数
	uint32_t roundCounter;			//记录状态机的轮询次数
	bufferUnion_t buffer;			//一个通用的buffer，用于存放与实际实用场景相关的数据
};

//初始化状态表
void fsm_init(stateMachine_t *pSm, uint8_t stateIDs_count, uint8_t stateID_default);
//复位状态机：将状态机的运行状态复位到默认状态
void fsm_reset(stateMachine_t *pSm);
//向指定的状态注册事件
void fsm_eventSingUp(stateMachine_t *pSm, uint8_t stateID, uint8_t nextState, stateMachine_eventResult_t (*pEventForGoing)(stateMachineUnit_t *));
void fsm_actionSignUp(stateMachine_t *pSm, uint8_t stateID, void (*pEnter)(stateMachineUnit_t *), void (*pDo)(stateMachineUnit_t *), void (*pExist)(stateMachineUnit_t *));

//运行一次指定的状态机
void fsm_run(stateMachine_t *pSm);

#endif /* C0FD9D79_317D_44BD_BF7F_E51B5C4F850C */
