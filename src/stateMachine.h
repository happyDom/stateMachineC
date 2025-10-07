#ifndef C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#define C0FD9D79_317D_44BD_BF7F_E51B5C4F850C
#include <stdint.h>

/*
 * 用户需要创建一个 userSMCfg.h 文件， 管理状态机和其中各状态是否包含数据buffer，以及数据buffer的类型，应包含如下内容：
//SM_BUFFER_NO		//状态机层面不定义buffer
//SM_BUFFER_FULL	//状态机层面定义全量buffer
//SM_BUFFER_PART	//状态机层面定义部分buffer
//SM_BUFFER_TINY	//状态机层面定义最小buffer
#define SM_BUFFER_NO

//ST_BUFFER_NO		//状态层面不定义buffer
//ST_BUFFER_FULL	//状态层面定义全量buffer
//ST_BUFFER_PART	//状态层面定义部分buffer
//ST_BUFFER_TINY	//状态层面定义最小buffer
#define ST_BUFFER_NO
*/
#include "userSMCfg.h"

typedef enum{
	false=0,
	true=!false
} _dyyBool;

#ifndef bool
#define bool _dyyBool
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define IS_NULL(p) (NULL == (p))
#define IS_pSafe(p) (NULL != (p))

typedef enum{
	aWait=0,
	go=1,
}smEventResult_t;

#if defined(SM_BUFFER_FULL) || defined(ST_BUFFER_FULL)
typedef struct {
	bool b;
	unsigned char ucAry[8];
	signed char sc;
    unsigned char uc;
	short s;
	unsigned short us;
    int i;
    unsigned int ui;
	long l;
	unsigned long ul;
    long long ll;
    unsigned long long ull;
    float f;
    double d;
	void *ptr;
} fullBuffer_t;
#elif defined(SM_BUFFER_PART) || defined(ST_BUFFER_PART)
typedef struct {
	union {
		bool b;
		signed char sc;
		unsigned char uc;
		short s;
		unsigned short us;
		unsigned char raw_16[2];
	}d16;
	
	union {
		int i;
    	unsigned int ui;
		long l;
		unsigned long ul;
		float f;
		unsigned char raw_32[4];
	}d32;
	
	union {
		long long ll;
    	unsigned long long ull;
		double d64;
		unsigned char raw_64[8];
	}d64;

	void *ptr;
} partBuffer_t;
#elif defined(SM_BUFFER_TINY) || defined(ST_BUFFER_TINY)
typedef struct {
	union {
		bool b;
		signed char sc;
		unsigned char uc;
		short s;
		unsigned short us;
		unsigned char raw_16[2];
	}d16;
} tinyBuffer_t;
#endif

struct stateMachine_event_s;
typedef struct stateMachineUnit_s smUnit_t;
typedef struct stateMachine_s stateMachine_t;

struct stateMachine_actionMap_s
{
	void (*pEnterAction)(smUnit_t *);
	void (*pDoAction)(smUnit_t *);
	void (*pExistAction)(smUnit_t *);
};

struct stateMachine_event_s
{
	smEventResult_t (*pEventForGoing)(smUnit_t *);
	uint8_t nextState;								//目标状态
	struct stateMachine_event_s *nextEvent;			//下一个事件
}; //这是一个单向链表,用于登记多个事件

struct stateMachineUnit_s
{
	bool latched;							//状态锁，为真时，状态机进行该状态的轮询时，不会检测该状态注册的事件
	uint8_t stateID_l;						//状态机的前一个状态
	uint8_t stateID;						//当前状态循环的状态
	struct stateMachine_actionMap_s actions;		//在本状态时需要执行的动作
	struct stateMachine_event_s *events;			//在本状态时，需要进行关注的事件，这是一个数组地址
	uint32_t roundCounter;					//这个计数器显示了在本状态期间，状态机轮询的次数，如果 1ms 轮询一次，支持最大 49.7 天时间的计数
	stateMachine_t *pSm;					//状态机的指针，这使得状态单元可以使用状态机中的信息
	
	//一个通用的buffer，用于存放与实际实用场景相关的数据
	#if defined(ST_BUFFER_FULL)
	fullBuffer_t buffer;
	#elif defined(ST_BUFFER_PART)
	partBuffer_t buffer;
	#elif defined(ST_BUFFER_TINY)
	tinyBuffer_t buffer;
	#endif
};

struct stateMachine_s
{
	bool latched;					//状态机锁，为真时，状态机不运行任何状态的动作，不检测任何事件
	uint32_t *enterCounterOf;		//一个数组，用于记录状态机中每一个状态出现的次数，在对应状态退出时进行计数
	smUnit_t *pSMChain;	//存放状态单元的数组空间的地址
	void (*actionOnChangeBeforeEnter)(smUnit_t *pstNew); //状态切换前要做的动作
	void (*actionAfterDo)(smUnit_t *); //在每个do事件后执行的动作
	uint8_t stateID;				//标记当前状态机的状态
	uint8_t stateID_default;		//状态机的默认状态
	uint8_t stateIDs_Count;			//状态机的总状态数
	uint32_t roundCounter;			//记录状态机的轮询次数

	// 定义一个buffer，用于存放与实际实用场景相关的数据
	#if defined(SM_BUFFER_FULL)
	fullBuffer_t buffer;
	#elif defined(SM_BUFFER_PART)
	partBuffer_t buffer;
	#elif defined(SM_BUFFER_TINY)
	tinyBuffer_t buffer;
	#endif

	//复位状态机：将状态机的运行状态复位到默认状态
	void (*reset)(stateMachine_t *pSm);
	//向指定的状态注册事件
	void (*eventSingUp)(stateMachine_t *pSm, uint8_t stateID, uint8_t nextState, smEventResult_t (*pEventForGoing)(smUnit_t *));
	void (*actionSignUp)(stateMachine_t *pSm, uint8_t stateID, void (*pEnter)(smUnit_t *), void (*pDo)(smUnit_t *), void (*pExist)(smUnit_t *));

	//运行一次指定的状态机
	void (*run)(stateMachine_t *pSm);
};

//初始化状态表
void fsm_init(stateMachine_t *pSm, uint8_t stateIDs_count, uint8_t stateID_default);
#endif
